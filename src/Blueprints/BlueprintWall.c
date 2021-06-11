#include "../Blueprint.h"
#include "../Main.h"
#include "../Box2DWrapper.h"
#include "../Box2DUtils.h"

void Wall_deinit(GameObject* obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->body);
}

int BlueprintWallInit(GameObject* obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->pos = position;
	obj->txSrc = (SDL_Rect){ 8 * TILE_WIDTH, 0, TILE_WIDTH, 2 * TILE_WIDTH };
	obj->txOffset = (Vec2F){ 0.0, -12.0 };
	obj->body = Box2DUtilsCreateStaticBox(obj, position, STATIC_CATEGORY, ((Vec2F) { 0.875, 0.125 }));
	obj->deinit = Wall_deinit;
	return 0;
}
