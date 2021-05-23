#include "Player.h"
#include "Main.h"

static void Player_prePhysics(Object *obj) {
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

int PlayerInit(Object *obj) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->prePhysics = Player_prePhysics;
	obj->txSrc = (SDL_Rect) {48, 0, 16, 16};
	obj->txSize = (Vec2F) {1.0, 1.0};

	Box2DBodyDef *bodyDef = Box2DBodyDefCreate();
	Box2DBodyDefSetTypeDynamic(bodyDef);
	Box2DBodyDefSetPosition(bodyDef, (Vec2F) {0.0, 0.0});
	Box2DBody *body = Box2DWorldCreateBody(CurrentWorld(), bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DPolygonShape *boxShape = Box2DPolygonShapeCreate();
	Box2DPolygonShapeSetAsBox(boxShape, (Vec2F) {0.5, 0.5});
	Box2DFixtureDef *fixtureDef = Box2DFixtureDefCreate();
	Box2DFixtureDefSetShape(fixtureDef, boxShape);
	Box2DFixtureDefSetDensity(fixtureDef, 1.0);
	Box2DFixtureDefSetFriction(fixtureDef, 0.3);
	Box2DFixture *fixture = Box2DBodyCreateFixtureFromFixtureDef(body, fixtureDef);
	Box2DFixtureDefDestroy(fixtureDef);
	Box2DPolygonShapeDestroy(boxShape);

	Box2DBodySetLinearDamping(body, 10.0);
	Box2DBodySetAngularDamping(body, 0.0);
	Box2DBodySetFixedRotation(body, true);
	obj->body = body;
	return 0;
}

void PlayerDeinit(Object *obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->body);
	ObjectDeinit(obj);
}
