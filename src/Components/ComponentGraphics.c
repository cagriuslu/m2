#include "../Component.h"
#include "../Object.h"
#include "../Game.h"
#include "../Def.h"

void ComponentGraphic_DefaultDraw(ComponentGraphic* gfx) {
	Object* obj = Pool_GetById(&GAME->objects, gfx->super.objId);
	Object* camera = Pool_GetById(&GAME->objects, GAME->cameraId);
	if (obj && camera && GAME->sdlTexture) {
		float scale = GAME->pixelsPerMeter / GAME->tileWidth;
		Vec2F obj_origin_wrt_camera_obj = Vec2F_Sub(obj->position, camera->position);
		Vec2I obj_origin_wrt_screen_center = Vec2I_From2F(Vec2F_Mul(obj_origin_wrt_camera_obj, GAME->pixelsPerMeter));
		Vec2I obj_gfx_origin_wrt_screen_center = Vec2I_Add(obj_origin_wrt_screen_center, (Vec2I) {
			-(int)round(gfx->center_px.x * scale),
			-(int)round(gfx->center_px.y * scale)
		});
		Vec2I obj_gfx_origin_wrt_screen_origin = Vec2I_Add((Vec2I) { GAME->windowRect.w / 2, GAME->windowRect.h / 2 }, obj_gfx_origin_wrt_screen_center);
		SDL_Rect dstrect = (SDL_Rect){
			obj_gfx_origin_wrt_screen_origin.x - (int)round(gfx->textureRect.w * scale / 2.0f),
			obj_gfx_origin_wrt_screen_origin.y - (int)round(gfx->textureRect.h * scale / 2.0f),
			(int)round(gfx->textureRect.w * scale),
			(int)round(gfx->textureRect.h * scale)
		};
		SDL_Point centerPoint = (SDL_Point){
			(int)round(gfx->center_px.x * scale) + dstrect.w/2 ,
			(int)round(gfx->center_px.y * scale) + dstrect.h/2
		};
		SDL_RenderCopyEx(GAME->sdlRenderer, GAME->sdlTexture, &gfx->textureRect, &dstrect, gfx->angle * 180.0 / M_PI, &centerPoint, SDL_FLIP_NONE);
	}
}

void ComponentGraphic_DefaultDrawHealthBar(ComponentGraphic* gfx, float healthRatio) {
	Object* obj = Game_FindObjectById(gfx->super.objId);
	Object* camera = Pool_GetById(&GAME->objects, GAME->cameraId);
	if (obj && camera) {
		float scale = GAME->pixelsPerMeter / GAME->tileWidth;
		Vec2F obj_origin_wrt_camera_obj = Vec2F_Sub(obj->position, camera->position);
		Vec2I obj_origin_wrt_screen_center = Vec2I_From2F(Vec2F_Mul(obj_origin_wrt_camera_obj, GAME->pixelsPerMeter));
		Vec2I obj_gfx_origin_wrt_screen_center = Vec2I_Add(obj_origin_wrt_screen_center, (Vec2I) {
			-(int)round(gfx->center_px.x * scale),
				-(int)round(gfx->center_px.y * scale)
		});
		Vec2I obj_gfx_origin_wrt_screen_origin = Vec2I_Add((Vec2I) { GAME->windowRect.w / 2, GAME->windowRect.h / 2 }, obj_gfx_origin_wrt_screen_center);
		SDL_Rect obj_gfx_dstrect = (SDL_Rect){
			obj_gfx_origin_wrt_screen_origin.x - (int)round(gfx->textureRect.w * scale / 2.0f),
			obj_gfx_origin_wrt_screen_origin.y - (int)round(gfx->textureRect.h * scale / 2.0f),
			(int)round(gfx->textureRect.w * scale),
			(int)round(gfx->textureRect.h * scale)
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

int ComponentGraphic_Init(ComponentGraphic* gfx, ID objectId) {
	memset(gfx, 0, sizeof(ComponentGraphic));
	XERR_REFLECT(Component_Init((Component*)gfx, objectId));
	gfx->draw = ComponentGraphic_DefaultDraw;
	return 0;
}

void ComponentGraphic_Term(ComponentGraphic* gfx) {
	Component_Term((Component*)gfx);
	memset(gfx, 0, sizeof(ComponentGraphic));
}

int ComponentGraphic_YComparatorCB(ID gfxIdA, ID gfxIdB) {
	ComponentGraphic* gfxA = Pool_GetById(&GAME->graphics, gfxIdA);
	ComponentGraphic* gfxB = Pool_GetById(&GAME->graphics, gfxIdB);
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
