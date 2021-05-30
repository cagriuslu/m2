#include "Object.h"
#include "Main.h"
#include <string.h>

void Object_ovrdGraphics(Object* obj) {
	if (obj->tx) {
		Object* camera = ArrayGet(CurrentObjectArray(), 1);

		Vec2F obj_origin_wrt_camera_obj = Vec2FSub(obj->pos, camera->pos);
		Vec2I obj_origin_wrt_screen_center = Vec2Fto2I(Vec2FMul(obj_origin_wrt_camera_obj, CurrentPixelsPerMeter()));
		Vec2I obj_gfx_origin_wrt_screen_center = Vec2IAdd(obj_origin_wrt_screen_center, (Vec2I) {
			obj->txOffset.x * CurrentPixelsPerMeter() / CurrentTileWidth(),
			obj->txOffset.y * CurrentPixelsPerMeter() / CurrentTileWidth()
		});
		Vec2I obj_gfx_origin_wrt_screen_origin = Vec2IAdd((Vec2I) { CurrentScreenWidth() / 2, CurrentScreenHeight() / 2 }, obj_gfx_origin_wrt_screen_center);
		SDL_Rect dstrect = (SDL_Rect){
			obj_gfx_origin_wrt_screen_origin.x - obj->txSrc.w * CurrentPixelsPerMeter() / CurrentTileWidth() / 2,
			obj_gfx_origin_wrt_screen_origin.y - obj->txSrc.h * CurrentPixelsPerMeter() / CurrentTileWidth() / 2,
			obj->txSrc.w * CurrentPixelsPerMeter() / CurrentTileWidth(),
			obj->txSrc.h * CurrentPixelsPerMeter() / CurrentTileWidth()
		};
		SDL_RenderCopyEx(CurrentRenderer(), obj->tx, &obj->txSrc, &dstrect, obj->angle, NULL, SDL_FLIP_NONE);
	}
}

int ObjectInit(Object *obj) {
	memset(obj, 0, sizeof(Object));
	obj->tx = CurrentTextureLUT();
	obj->ovrdGraphics = Object_ovrdGraphics;
	return 0;
}

void ObjectDeinit(Object *obj) {
	if (obj->deinit) {
		obj->deinit(obj);
	}
}
