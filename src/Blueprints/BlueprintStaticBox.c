#include "../Blueprint.h"
#include "../Main.h"
#include "../Box2DWrapper.h"
#include "../Box2DUtils.h"
#include <stdio.h>

void StaticBox_deinit(GameObject* obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->body);
}

int BlueprintStaticBoxInit(GameObject *obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->pos = position;
	obj->txSrc = (SDL_Rect) {TILE_WIDTH, 4 * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
	obj->txOffset = (Vec2F){ 0.0, -4.5 };
	obj->body = Box2DUtilsCreateStaticBox(obj, position, STATIC_CATEGORY, ((Vec2F) { 0.875, 0.125 }));
	obj->deinit = StaticBox_deinit;
	return 0;
}
