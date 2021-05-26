#include "God.h"
#include "Error.h"
#include "Main.h"
#include "Event.h"

static void God_prePhysics(Object *obj) {
	if (IsKeyDown(KEY_UP)) {
		Box2DBodyApplyForceToCenter(obj->body, (Vec2F) {0.0, -100.0}, true);
	}
	if (IsKeyDown(KEY_DOWN)) {
		Box2DBodyApplyForceToCenter(obj->body, (Vec2F) {0.0, 100.0}, true);
	}
	if (IsKeyDown(KEY_LEFT)) {
		Box2DBodyApplyForceToCenter(obj->body, (Vec2F) {-100.0, 0.0}, true);
	}
	if (IsKeyDown(KEY_RIGHT)) {
		Box2DBodyApplyForceToCenter(obj->body, (Vec2F) {100.0, 0.0}, true);
	}
}

int GodInit(Object *obj) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->prePhysics = God_prePhysics;
	return 0;
}

void GodDeinit(Object *obj) {
	ObjectDeinit(obj);
}