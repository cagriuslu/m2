#include "God.h"
#include "Error.h"
#include "Main.h"
#include "Event.h"
#include "Box2DUtils.h"

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

void God_deinit(Object* obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->body);
}

int GodInit(Object *obj) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->prePhysics = God_prePhysics;
	obj->body = Box2DUtilsCreateDynamicDisk(
		(Vec2F) {0, 0}, // Position
		DONT_SLEEP, // Allow sleep
		IS_SENSOR, // Is sensor?
		0.25, // Radius
		10.0, // Density
		10.0 // Linear Damping
	);
	obj->deinit = God_deinit;
	return 0;
}
