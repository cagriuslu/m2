#include "../Component.h"
#include "../Object.h"
#include "../Main.h"
#include "../Error.h"
#include "../Log.h"
#include <string.h>

void GraphicsComponent_DefaultDraw(ComponentGraphics* gfx) {
	Object* obj = Pool_GetById(&GAME->objects, gfx->super.objId);
	Object* camera = Pool_GetById(&GAME->objects, GAME->cameraId);
	if (obj && camera && GAME->sdlTexture) {
		float scale = GAME->pixelsPerMeter / GAME->tileWidth;
		Vec2F obj_origin_wrt_camera_obj = Vec2F_Sub(obj->position, camera->position);
		Vec2I obj_origin_wrt_screen_center = Vec2F_To2I(Vec2F_Mul(obj_origin_wrt_camera_obj, GAME->pixelsPerMeter));
		Vec2I obj_gfx_origin_wrt_screen_center = Vec2I_Add(obj_origin_wrt_screen_center, (Vec2I) {
			-(int)round(gfx->txCenter.x * scale),
			-(int)round(gfx->txCenter.y * scale)
		});
		Vec2I obj_gfx_origin_wrt_screen_origin = Vec2I_Add((Vec2I) { GAME->windowWidth / 2, GAME->windowHeight / 2 }, obj_gfx_origin_wrt_screen_center);
		SDL_Rect dstrect = (SDL_Rect){
			obj_gfx_origin_wrt_screen_origin.x - (int)round(gfx->txSrc.w * scale / 2.0f),
			obj_gfx_origin_wrt_screen_origin.y - (int)round(gfx->txSrc.h * scale / 2.0f),
			(int)round(gfx->txSrc.w * scale),
			(int)round(gfx->txSrc.h * scale)
		};
		SDL_Point centerPoint = (SDL_Point){
			(int)round(gfx->txCenter.x * scale) + dstrect.w/2 ,
			(int)round(gfx->txCenter.y * scale) + dstrect.h/2
		};
		SDL_RenderCopyEx(GAME->sdlRenderer, GAME->sdlTexture, &gfx->txSrc, &dstrect, gfx->txAngle * 180.0 / M_PI, &centerPoint, SDL_FLIP_NONE);
	}
}

void GraphicsComponent_DefaultDrawHealthBar(ComponentGraphics* gfx, float healthRatio) {
	Object* obj = FindObjectOfComponent(gfx);
	Object* camera = Pool_GetById(&GAME->objects, GAME->cameraId);
	if (obj && camera) {
		float scale = GAME->pixelsPerMeter / GAME->tileWidth;
		Vec2F obj_origin_wrt_camera_obj = Vec2F_Sub(obj->position, camera->position);
		Vec2I obj_origin_wrt_screen_center = Vec2F_To2I(Vec2F_Mul(obj_origin_wrt_camera_obj, GAME->pixelsPerMeter));
		Vec2I obj_gfx_origin_wrt_screen_center = Vec2I_Add(obj_origin_wrt_screen_center, (Vec2I) {
			-(int)round(gfx->txCenter.x * scale),
				-(int)round(gfx->txCenter.y * scale)
		});
		Vec2I obj_gfx_origin_wrt_screen_origin = Vec2I_Add((Vec2I) { GAME->windowWidth / 2, GAME->windowHeight / 2 }, obj_gfx_origin_wrt_screen_center);
		SDL_Rect obj_gfx_dstrect = (SDL_Rect){
			obj_gfx_origin_wrt_screen_origin.x - (int)round(gfx->txSrc.w * scale / 2.0f),
			obj_gfx_origin_wrt_screen_origin.y - (int)round(gfx->txSrc.h * scale / 2.0f),
			(int)round(gfx->txSrc.w * scale),
			(int)round(gfx->txSrc.h * scale)
		};

		int healthBarWidth = obj_gfx_dstrect.w * 8 / 10;

		SDL_Rect filled_dstrect = (SDL_Rect){
			obj_gfx_dstrect.x + (obj_gfx_dstrect.w - healthBarWidth) / 2,
			obj_gfx_dstrect.y + obj_gfx_dstrect.h,
			(int)round(healthBarWidth * healthRatio),
			GAME->tileWidth / 6
		};
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 255, 0, 0, 200);
		SDL_RenderFillRect(GAME->sdlRenderer, &filled_dstrect);

		SDL_Rect empty_dstrect = (SDL_Rect){
			filled_dstrect.x + filled_dstrect.w,
			filled_dstrect.y,
			healthBarWidth - filled_dstrect.w,
			filled_dstrect.h
		};
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 127, 0, 0, 200);
		SDL_RenderFillRect(GAME->sdlRenderer, &empty_dstrect);
	}
}

int GraphicsComponent_Init(ComponentGraphics* gfx, ID objectId) {
	memset(gfx, 0, sizeof(ComponentGraphics));
	REFLECT_ERROR(Component_Init((Component*)gfx, objectId));
	gfx->draw = GraphicsComponent_DefaultDraw;
	return 0;
}

void GraphicsComponent_Term(ComponentGraphics* gfx) {
	Component_Term((Component*)gfx);
	memset(gfx, 0, sizeof(ComponentGraphics));
}

int GraphicsComponent_YComparatorCB(ID gfxIdA, ID gfxIdB) {
	ComponentGraphics* gfxA = Pool_GetById(&GAME->graphics, gfxIdA);
	ComponentGraphics* gfxB = Pool_GetById(&GAME->graphics, gfxIdB);
	if (gfxA && gfxB) {
		Object* a = Pool_GetById(&GAME->objects, gfxA->super.objId);
		Object* b = Pool_GetById(&GAME->objects, gfxB->super.objId);
		if (a && b) {
			float diff = b->position.y - a->position.y;
			if (0 < diff) {
				return 1;
			} else if (diff < 0) {
				return -1;
			} else {
				return 0;
			}
		}
	}
	return 0;
}
