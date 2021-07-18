#include "../Component.h"
#include "../Object.h"
#include "../Main.h"
#include "../Error.h"
#include "../Log.h"
#include <string.h>

void GraphicsComponent_draw(ComponentGraphics* gfx) {
	Level* level = CurrentLevel();
	Object* obj = Bucket_GetById(&level->objects, gfx->super.objId);
	Object* camera = Bucket_GetById(&level->objects, level->cameraId);
	if (obj && camera && gfx->tx) {
		float scale = CurrentPixelsPerMeter() / CurrentTileWidth();
		Vec2F obj_origin_wrt_camera_obj = Vec2F_Sub(obj->position, camera->position);
		Vec2I obj_origin_wrt_screen_center = Vec2F_To2I(Vec2F_Mul(obj_origin_wrt_camera_obj, CurrentPixelsPerMeter()));
		Vec2I obj_gfx_origin_wrt_screen_center = Vec2I_Add(obj_origin_wrt_screen_center, (Vec2I) {
			-(int)round(gfx->txCenter.x * scale),
			-(int)round(gfx->txCenter.y * scale)
		});
		Vec2I obj_gfx_origin_wrt_screen_origin = Vec2I_Add((Vec2I) { CurrentScreenWidth() / 2, CurrentScreenHeight() / 2 }, obj_gfx_origin_wrt_screen_center);
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
		SDL_RenderCopyEx(CurrentRenderer(), gfx->tx, &gfx->txSrc, &dstrect, gfx->txAngle * 180.0 / M_PI, &centerPoint, SDL_FLIP_NONE);
	}
}

int GraphicsComponent_Init(ComponentGraphics* gfx, ID objectId) {
	memset(gfx, 0, sizeof(ComponentGraphics));
	PROPAGATE_ERROR(Component_Init((Component*)gfx, objectId));
	gfx->tx = CurrentTextureLUT();
	gfx->draw = GraphicsComponent_draw;
	return 0;
}

void GraphicsComponent_Term(ComponentGraphics* gfx) {
	Component_Term((Component*)gfx);
	memset(gfx, 0, sizeof(ComponentGraphics));
}

int GraphicsComponent_YComparatorCB(ID gfxIdA, ID gfxIdB) {
	Level* level = CurrentLevel();
	ComponentGraphics* gfxA = Bucket_GetById(&level->graphics, gfxIdA);
	ComponentGraphics* gfxB = Bucket_GetById(&level->graphics, gfxIdB);
	if (gfxA && gfxB) {
		Object* a = Bucket_GetById(&level->objects, gfxA->super.objId);
		Object* b = Bucket_GetById(&level->objects, gfxB->super.objId);
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
