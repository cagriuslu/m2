#include "../Blueprint.h"
#include "../Main.h"
#include "../Box2DWrapper.h"
#include "../Box2DUtils.h"

void Skeleton_deinit(Object* obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->body);
}

int BlueprintSkeletonInit(Object* obj, Vec2F position) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->pos = position;
	obj->txSrc = (SDL_Rect){2 * TILE_WIDTH, 0, TILE_WIDTH, TILE_WIDTH };
	obj->txOffset = (Vec2F){ 0.0, -4.5 };
	obj->body = Box2DUtilsCreateDynamicDisk(
		obj,
		position,
		ALLOW_SLEEP,
		ENEMY_CATEGORY,
		0.2083, // Radius
		10, // Mass
		10.0 // Damping
	);
	obj->deinit = Skeleton_deinit;
	return 0;
}
