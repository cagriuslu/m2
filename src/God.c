#include "God.h"
#include "Error.h"
#include "Main.h"
#include "EventHandling.h"

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

	Box2DBodyDef *bodyDef = Box2DBodyDefCreate();
	Box2DBodyDefSetTypeDynamic(bodyDef);
	Box2DBodyDefSetPosition(bodyDef, (Vec2F) {0.0, 0.0});
	Box2DBody *body = Box2DWorldCreateBody(CurrentWorld(), bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DPolygonShape *boxShape = Box2DPolygonShapeCreate();
	Box2DPolygonShapeSetAsBox(boxShape, (Vec2F) {0.25, 0.125}); // Should somewhat mirror Player
	Box2DFixtureDef *fixtureDef = Box2DFixtureDefCreate();
	Box2DFixtureDefSetShape(fixtureDef, boxShape);
	Box2DFixtureDefSetDensity(fixtureDef, 16.0);
	Box2DFixtureDefSetFriction(fixtureDef, 0.0);
	Box2DFixture *fixture = Box2DBodyCreateFixtureFromFixtureDef(body, fixtureDef);
	Box2DFixtureDefDestroy(fixtureDef);
	Box2DPolygonShapeDestroy(boxShape);
	Box2DFixtureSetSensor(fixture, true);

	Box2DBodySetLinearDamping(body, 10.0);
	Box2DBodySetAngularDamping(body, 0.0);
	Box2DBodySetFixedRotation(body, true);
	obj->body = body;
	return 0;
}

void GodDeinit(Object *obj) {
	ObjectDeinit(obj);
}