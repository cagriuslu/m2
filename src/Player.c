#include "Player.h"
#include "Main.h"
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
	obj->prePhysics = Player_prePhysics;
	obj->txSrc = (SDL_Rect){ 3 * TILE_WIDTH, 0, TILE_WIDTH, TILE_WIDTH };
	obj->txOffset = (Vec2F) {0.0, -9.75};

	Box2DBodyDef *bodyDef = Box2DBodyDefCreate();
	Box2DBodyDefSetTypeDynamic(bodyDef);
	Box2DBodyDefSetPosition(bodyDef, (Vec2F) {0.0, 0.0});
	Box2DBody *body = Box2DWorldCreateBody(CurrentWorld(), bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DCircleShape *circleShape = Box2DCircleShapeCreate();
	Box2DCircleShapeSetRadius(circleShape, 0.21875);

	Box2DFixtureDef *fixtureDef = Box2DFixtureDefCreate();
	Box2DFixtureDefSetShape(fixtureDef, circleShape);
	Box2DFixtureDefSetDensity(fixtureDef, 15.0);
	Box2DFixtureDefSetFriction(fixtureDef, 0.05);
	Box2DFixture *fixture = Box2DBodyCreateFixtureFromFixtureDef(body, fixtureDef);
	Box2DFixtureDefDestroy(fixtureDef);
	Box2DCircleShapeDestroy(circleShape);

	Box2DBodySetLinearDamping(body, 10.0);
	Box2DBodySetAngularDamping(body, 0.0);
	Box2DBodySetFixedRotation(body, true);
	obj->body = body;

	obj->deinit = Player_deinit;
	return 0;
}
