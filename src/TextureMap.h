#ifndef TEXTURE_MAP_H
#define TEXTURE_MAP_H

#include "HashMap.h"
#include "Vec2F.h"
#include <SDL.h>

typedef struct _TextureMap {
	SDL_Texture* map;
	HashMapOfTextures lut;
} TextureMap;

typedef struct _Texture {
	SDL_Texture* map; // weak link
	SDL_Rect rect;
	Vec2F center;
	float diskRadius; // in meters
	Vec2F boxDims; // in meters
	Vec2F boxCenterOffset; // in meters, w.r.t. Texture->center
} Texture;

XErr TextureMap_Init(TextureMap* tm, unsigned pixelsPerMeter, const char* imageFile, const char* metaImageFile, const char* metaFile);
void TextureMap_Term(TextureMap* tm);

#endif
