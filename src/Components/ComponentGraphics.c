#include "../Component.h"
#include "../Object.h"
#include "../Main.h"
#include "../Error.h"
#include <string.h>

void GraphicsComponent_draw(GraphicsComponent* gfx) {
	Level* level = CurrentLevel();
	Object* obj = BucketGetById(&level->objects, gfx->super.objId);
	Object* camera = BucketGetById(&level->objects, level->cameraId);
	if (obj && camera && gfx->tx) {
		Vec2F obj_origin_wrt_camera_obj = Vec2FSub(obj->position, camera->position);
		Vec2I obj_origin_wrt_screen_center = Vec2Fto2I(Vec2FMul(obj_origin_wrt_camera_obj, CurrentPixelsPerMeter()));
		Vec2I obj_gfx_origin_wrt_screen_center = Vec2IAdd(obj_origin_wrt_screen_center, (Vec2I) {
			(int)round(gfx->txOffset.x * CurrentPixelsPerMeter()) / CurrentTileWidth(),
				(int)round(gfx->txOffset.y * CurrentPixelsPerMeter()) / CurrentTileWidth()
		});
		Vec2I obj_gfx_origin_wrt_screen_origin = Vec2IAdd((Vec2I) { CurrentScreenWidth() / 2, CurrentScreenHeight() / 2 }, obj_gfx_origin_wrt_screen_center);
		SDL_Rect dstrect = (SDL_Rect){
			obj_gfx_origin_wrt_screen_origin.x - (int)round(gfx->txSrc.w * CurrentPixelsPerMeter()) / CurrentTileWidth() / 2,
			obj_gfx_origin_wrt_screen_origin.y - (int)round(gfx->txSrc.h * CurrentPixelsPerMeter()) / CurrentTileWidth() / 2,
			(int)round(gfx->txSrc.w * CurrentPixelsPerMeter()) / CurrentTileWidth(),
			(int)round(gfx->txSrc.h * CurrentPixelsPerMeter()) / CurrentTileWidth()
		};
		SDL_RenderCopyEx(CurrentRenderer(), gfx->tx, &gfx->txSrc, &dstrect, gfx->txAngle * 180.0 / M_PI, NULL, SDL_FLIP_NONE);
	}
}

int GraphicsComponentInit(GraphicsComponent* gfx, uint64_t objectId) {
	memset(gfx, 0, sizeof(GraphicsComponent));
	PROPAGATE_ERROR(ComponentInit((Component*)gfx, objectId));
	gfx->tx = CurrentTextureLUT();
	gfx->draw = GraphicsComponent_draw;
	return 0;
}

void GraphicsComponentDeinit(GraphicsComponent* gfx) {
	ComponentDeinit((Component*)gfx);
	memset(gfx, 0, sizeof(GraphicsComponent));
}

int GraphicsComponentYComparatorCB(uint64_t gfxIdA, uint64_t gfxIdB) {
	Level* level = CurrentLevel();
	GraphicsComponent* gfxA = BucketGetById(&level->graphics, gfxIdA);
	GraphicsComponent* gfxB = BucketGetById(&level->graphics, gfxIdB);
	if (gfxA && gfxB) {
		Object* a = BucketGetById(&level->objects, gfxA->super.objId);
		Object* b = BucketGetById(&level->objects, gfxB->super.objId);
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
