#include "Box2DUtils.h"
#include "Main.h"

static Box2DBody* CreateBody(bool isDynamic, Vec2F position, bool allowSleep) {
	Box2DBodyDef* bodyDef = Box2DBodyDefCreate();
	if (isDynamic) {
		Box2DBodyDefSetTypeDynamic(bodyDef);
	}
	Box2DBodyDefSetPosition(bodyDef, position);
	Box2DBodyDefSetAllowSleep(bodyDef, allowSleep);
	Box2DBody* body = Box2DWorldCreateBody(CurrentWorld(), bodyDef);
	Box2DBodyDefDestroy(bodyDef);
	return body;
}

Box2DBody* Box2DUtilsCreateStaticBox(Vec2F position, bool allowSleep, bool isSensor, Vec2F dims) {
	Box2DBody* body = CreateBody(false, position, allowSleep);
	
	Box2DPolygonShape* boxShape = Box2DPolygonShapeCreate();
	Box2DPolygonShapeSetAsBox(boxShape, Vec2FMul(dims, 0.5)); // convert to half dims
	Box2DFixture* fixture = Box2DBodyCreateFixtureFromShape(body, boxShape, 0.0);
	Box2DFixtureSetSensor(fixture, isSensor);

	Box2DPolygonShapeDestroy(boxShape);
	return body;
}

Box2DBody* Box2DUtilsCreateDynamicBox(Vec2F position, bool allowSleep, bool isSensor, Vec2F dims, float density, float linearDamping) {
	Box2DBody* body = CreateBody(true, position, allowSleep);

	Box2DPolygonShape* boxShape = Box2DPolygonShapeCreate();
	Box2DPolygonShapeSetAsBox(boxShape, Vec2FMul(dims, 0.5)); // convert to half dims

	Box2DFixtureDef* fixtureDef = Box2DFixtureDefCreate();
	Box2DFixtureDefSetShape(fixtureDef, boxShape);
	Box2DFixtureDefSetDensity(fixtureDef, density);
	Box2DFixtureDefSetFriction(fixtureDef, 0.05);
	Box2DFixture* fixture = Box2DBodyCreateFixtureFromFixtureDef(body, fixtureDef);
	Box2DFixtureSetSensor(fixture, isSensor);

	Box2DBodySetLinearDamping(body, linearDamping);
	Box2DBodySetAngularDamping(body, 0.0);
	Box2DBodySetFixedRotation(body, true);

	Box2DFixtureDefDestroy(fixtureDef);
	Box2DPolygonShapeDestroy(boxShape);
	return body;
}

Box2DBody* Box2DUtilsCreateStaticDisk(Vec2F position, bool allowSleep, bool isSensor, float radius) {
	Box2DBody* body = CreateBody(false, position, allowSleep);

	Box2DCircleShape* circleShape = Box2DCircleShapeCreate();
	Box2DCircleShapeSetRadius(circleShape, radius);
	Box2DFixture* fixture = Box2DBodyCreateFixtureFromShape(body, circleShape, 0.0);

	Box2DFixtureSetSensor(fixture, isSensor);
	Box2DCircleShapeDestroy(circleShape);
	return body;
}

Box2DBody* Box2DUtilsCreateDynamicDisk(Vec2F position, bool allowSleep, bool isSensor, float radius, float density, float linearDamping) {
	Box2DBody* body = CreateBody(true, position, allowSleep);
	
	Box2DCircleShape* circleShape = Box2DCircleShapeCreate();
	Box2DCircleShapeSetRadius(circleShape, radius);

	Box2DFixtureDef* fixtureDef = Box2DFixtureDefCreate();
	Box2DFixtureDefSetShape(fixtureDef, circleShape);
	Box2DFixtureDefSetDensity(fixtureDef, density);
	Box2DFixtureDefSetFriction(fixtureDef, 0.05);
	Box2DFixture* fixture = Box2DBodyCreateFixtureFromFixtureDef(body, fixtureDef);
	Box2DFixtureSetSensor(fixture, isSensor);
	
	Box2DBodySetLinearDamping(body, linearDamping);
	Box2DBodySetAngularDamping(body, 0.0);
	Box2DBodySetFixedRotation(body, true);

	Box2DFixtureDefDestroy(fixtureDef);
	Box2DCircleShapeDestroy(circleShape);
	return body;
}
