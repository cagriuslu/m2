#include "TextureMap.h"
#include "Log.h"
#include "Main.h"
#include "Txt.h"
#include "SDLUtils.h"
#include <SDL_image.h>
#include <string.h>

ArrayOfVec2Is FindPixels(SDL_Surface* surf, SDL_Rect rect, uint8_t r, uint8_t g, uint8_t b);
Vec2F FindCenterOfPixels(ArrayOfVec2Is* points);

XErr TextureMap_Init(TextureMap* tm, unsigned pixelsPerMeter, const char* imageFile, const char* metaImageFile, const char* metaFile) {
	memset(tm, 0, sizeof(TextureMap));
	SDL_Surface* image = IMG_Load(imageFile);
	if (!image) {
		LOGXV_ERR(XERR_FILE_NOT_FOUND, String, imageFile);
		return XERR_FILE_NOT_FOUND;
	}
	tm->map = SDL_CreateTextureFromSurface(GAME->sdlRenderer, image);
	if (!tm->map) {
		LOG_ERR("Unable to convert surface to texture");
		SDL_FreeSurface(image);
		return XERR_SDL_ERROR;
	}
	SDL_Surface* metaImage = IMG_Load(metaImageFile);
	if (!metaImage) {
		LOGXV_ERR(XERR_FILE_NOT_FOUND, String, metaImageFile);
		SDL_DestroyTexture(tm->map);
		SDL_FreeSurface(image);
		return XERR_FILE_NOT_FOUND;
	}
	Txt meta;
	XErr result = Txt_InitFromFile(&meta, metaFile);
	if (result) {
		SDL_FreeSurface(metaImage);
		SDL_DestroyTexture(tm->map);
		SDL_FreeSurface(image);
		return result;
	}
	result = HashMap_Init(&tm->lut, sizeof(Texture), NULL);
	if (result) {
		Txt_Term(&meta);
		SDL_FreeSurface(metaImage);
		SDL_DestroyTexture(tm->map);
		SDL_FreeSurface(image);
		return result;
	}

	// Iterate over definitions in the file
	for (size_t textureDefIdx = 0; textureDefIdx < Array_Length(&meta.txtKVPairs); textureDefIdx++) {
		TxtKV* textureDefKV = Array_Get(&meta.txtKVPairs, textureDefIdx);
		
		Texture* texture = HashMap_SetStringKey(&tm->lut, textureDefKV->key, NULL);
		memset(texture, 0, sizeof(Texture));
		texture->map = tm->map;

		char* xStr = TxtKV_DuplicateUrlEncodedValue(textureDefKV, "x=");
		char* yStr = TxtKV_DuplicateUrlEncodedValue(textureDefKV, "y=");
		char* wStr = TxtKV_DuplicateUrlEncodedValue(textureDefKV, "w=");
		char* hStr = TxtKV_DuplicateUrlEncodedValue(textureDefKV, "h=");
		texture->rect.x = atoi(xStr);
		texture->rect.y = atoi(yStr);
		texture->rect.w = atoi(wStr);
		texture->rect.h = atoi(hStr);
		free(hStr);
		free(wStr);
		free(yStr);
		free(xStr);

		const Vec2F originPoint = Vec2F_FromVec2I(SDLUtils_CenterOfRect(texture->rect));

		// Look for red pixels for center
		ArrayOfVec2Is redPixels = FindPixels(metaImage, texture->rect, 255, 0, 0);
		if (Array_Length(&redPixels)) {
			const Vec2F redPoint = FindCenterOfPixels(&redPixels);
			const Vec2F redPointWRTOrigin = Vec2F_Sub(redPoint, originPoint);
			texture->center = redPointWRTOrigin;
		}
		Array_Term(&redPixels);

		// Look for disk collider
		ArrayOfVec2Is bluePixels = FindPixels(metaImage, texture->rect, 0, 0, 255);
		if (Array_Length(&bluePixels)) {
			const Vec2F bluePoint = FindCenterOfPixels(&bluePixels);
			const Vec2F bluePointWRTOrigin = Vec2F_Sub(bluePoint, originPoint);
			texture->diskRadius = Vec2F_Distance(bluePointWRTOrigin, texture->center) / (float)pixelsPerMeter;
		} else {
			// Look for box collider
			ArrayOfVec2Is green255Pixels = FindPixels(metaImage, texture->rect, 0, 255, 0);
			ArrayOfVec2Is green254Pixels = FindPixels(metaImage, texture->rect, 0, 254, 0);
			if (Array_Length(&green255Pixels) && Array_Length(&green254Pixels)) {
				const Vec2F green255Point = FindCenterOfPixels(&green255Pixels);
				const Vec2F green254Point = FindCenterOfPixels(&green254Pixels);
				texture->boxDims = Vec2F_Div(Vec2F_Distance2(green255Point, green254Point), (float)pixelsPerMeter);
				const Vec2F greenCenterPoint = Vec2F_Div(Vec2F_Add(green255Point, green254Point), 2.0f);
				const Vec2F greenCenterPointWRTOrigin = Vec2F_Sub(greenCenterPoint, originPoint);
				const Vec2F greenCenterPointWRTTextureCenter = Vec2F_Sub(greenCenterPointWRTOrigin, texture->center);
				texture->boxCenterOffset = Vec2F_Div(greenCenterPointWRTTextureCenter, (float)pixelsPerMeter);
			}
			Array_Term(&green255Pixels);
			Array_Term(&green254Pixels);
		}
		Array_Term(&bluePixels);
	}
	
	Txt_Term(&meta);
	SDL_FreeSurface(metaImage);
	SDL_FreeSurface(image);
	return 0;
}

void TextureMap_Term(TextureMap* tm) {
	HashMap_Term(&tm->lut);
	SDL_DestroyTexture(tm->map);
	memset(tm, 0, sizeof(TextureMap));
}

ArrayOfVec2Is FindPixels(SDL_Surface* surf, SDL_Rect rect, uint8_t r, uint8_t g, uint8_t b) {
	ArrayOfVec2Is array;
	Array_Init(&array, sizeof(Vec2I), 1, UINT32_MAX, NULL);

	if (SDL_MUSTLOCK(surf)) {
		SDL_LockSurface(surf);
	}

	const unsigned bytesPerPixel = surf->format->BytesPerPixel;
	for (int y = rect.y; y < (rect.y + rect.h); y++) {
		for (int x = rect.x; x < (rect.x + rect.w); x++) {
			uint8_t* pixelPtr = (uint8_t*)surf->pixels + y * surf->pitch + x * bytesPerPixel;
			uint32_t pixel = *(uint32_t*)pixelPtr;
			SDL_Color color;
			SDL_GetRGBA(pixel, surf->format, &color.r, &color.g, &color.b, &color.a);
			if (color.r == r && color.g == g && color.b == b) {
				Vec2I point = (Vec2I){x, y};
				Array_Append(&array, &point);
			}
		}
	}

	if (SDL_MUSTLOCK(surf)) {
		SDL_UnlockSurface(surf);
	}

	return array;
}

Vec2F FindCenterOfPixels(ArrayOfVec2Is* points) {
	Vec2F center = VEC2F_ZERO;
	for (size_t i = 0; i < Array_Length(points); i++) {
		Vec2I* point = Array_Get(points, i);
		center = Vec2F_Add(center, Vec2F_FromVec2I(*point));
	}
	return Vec2F_Div(center, (float)Array_Length(points));
}
