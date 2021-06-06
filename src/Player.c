#include "Player.h"
#include "Box2DUtils.h"
#include "Main.h"
#include "Object.h"
#include "Event.h"
#include <math.h>

// Shoot with mouse primary and secondary
// Change primary skill with wheel
// Change secondary skill with middle button
// Shift button is also used during combat
// Ctrl and Alt should not be used during combat

static void Player_prePhysics(Object *obj) {
	// if (IsButtonDown(BUTTON_SECONDARY)) {
	// 	Vec2I pos_wrt_screen_origin = PointerPosition();
	// 	Vec2I pos_wrt_screen_center = (Vec2I) {
	// 		pos_wrt_screen_origin.x - CurrentScreenWidth() / 2,
	// 		pos_wrt_screen_origin.y - CurrentScreenHeight() / 2
	// 	};
	// 	float length = sqrt(pow((float) pos_wrt_screen_center.x, 2) + pow((float) pos_wrt_screen_center.y, 2));
	// 	Vec2F unit_vector = (Vec2F) {pos_wrt_screen_center.x / length, pos_wrt_screen_center.y / length};
	// 	Vec2F force = Vec2FMul(unit_vector, 100.0);
	// 	Box2DBodyApplyForceToCenter(obj->body, force, true);
	// }

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

void Player_deinit(Object* obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->body);
}

int PlayerInit(Object *obj) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->type = OBJECT_PLAYER_BASIC;
	obj->prePhysics = Player_prePhysics;
	obj->txSrc = (SDL_Rect){ 3 * TILE_WIDTH, 0, TILE_WIDTH, TILE_WIDTH };
	obj->txOffset = (Vec2F) {0.0, -6.5};
	obj->body = Box2DUtilsCreateDynamicDisk(
		obj,
		((Vec2F) {0, 0}), // Position
		DONT_SLEEP,
		PLAYER_CATEGORY,
		0.229167f, // Radius
		4.0f, // Mass
		10.0f // Damping
	);
	obj->deinit = Player_deinit;
	return 0;
}
