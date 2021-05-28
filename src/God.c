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

void God_deinit(Object* obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->body);
}

int GodInit(Object *obj) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->prePhysics = God_prePhysics;

	Box2DBodyDef* bodyDef = Box2DBodyDefCreate();
	Box2DBodyDefSetTypeDynamic(bodyDef);
	Box2DBodyDefSetPosition(bodyDef, (Vec2F) { 0.0, 0.0 });
	Box2DBody* body = Box2DWorldCreateBody(CurrentWorld(), bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DCircleShape* circleShape = Box2DCircleShapeCreate();
	Box2DCircleShapeSetRadius(circleShape, 0.25);

	Box2DFixtureDef* fixtureDef = Box2DFixtureDefCreate();
	Box2DFixtureDefSetShape(fixtureDef, circleShape);
	Box2DFixtureDefSetDensity(fixtureDef, 10.0);
	Box2DFixtureDefSetFriction(fixtureDef, 0.05);
	Box2DFixture* fixture = Box2DBodyCreateFixtureFromFixtureDef(body, fixtureDef);
	Box2DFixtureSetSensor(fixture, true);
	Box2DFixtureDefDestroy(fixtureDef);
	Box2DCircleShapeDestroy(circleShape);

	Box2DBodySetLinearDamping(body, 10.0);
	Box2DBodySetAngularDamping(body, 0.0);
	Box2DBodySetFixedRotation(body, true);
	obj->body = body;

	obj->deinit = God_deinit;
	return 0;
}
