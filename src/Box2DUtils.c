#include "Box2DUtils.h"
#include "Main.h"

Box2DBody* Box2DUtilsCreateBody(uint64_t phyId, bool isDisk, bool isDynamic, Vec2F position, bool allowSleep, bool isBullet, bool isSensor, uint16_t categoryBits, uint16_t maskBits, Vec2F boxDims, float diskRadius, float mass, float linearDamping) {
	Box2DBodyDef* bodyDef = Box2DBodyDefCreate();
	if (isDynamic) {
		Box2DBodyDefSetTypeDynamic(bodyDef);
	}
	Box2DBodyDefSetPosition(bodyDef, position);
	Box2DBodyDefSetAllowSleep(bodyDef, allowSleep);
	Box2DBodyDefSetUserData(bodyDef, (void*) ((uintptr_t) phyId));
	Box2DBodyDefSetBullet(bodyDef, isBullet);
	Box2DBody* body = Box2DWorldCreateBody(CurrentLevel()->world, bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DShape* shape = NULL;
	if (isDisk) {
		Box2DCircleShape* circleShape = Box2DCircleShapeCreate();
		Box2DCircleShapeSetRadius(circleShape, diskRadius);
		shape = circleShape;
	} else {
		Box2DPolygonShape* boxShape = Box2DPolygonShapeCreate();
		Box2DPolygonShapeSetAsBox(boxShape, Vec2FMul(boxDims, 0.5)); // convert to half dims
		shape = boxShape;
	}
	Box2DFixtureDef* fixtureDef = Box2DFixtureDefCreate();
	Box2DFixtureDefSetShape(fixtureDef, shape);
	Box2DFixtureDefSetFriction(fixtureDef, 0.05f);
	Box2DFixtureDefSetCategoryBits(fixtureDef, categoryBits);
	// If mask is not provided, infer it
	if (maskBits == 0) {
		switch (categoryBits)
		{
			case STATIC_OBJECT_CATEGORY:
				maskBits = STATIC_OBJECT_CATEGORY | STATIC_CLIFF_CATEGORY | PLAYER_CATEGORY | PLAYER_BULLET_CATEGORY | ENEMY_CATEGORY | ENEMY_BULLET_CATEGORY;
				break;
			case STATIC_CLIFF_CATEGORY:
				maskBits = STATIC_OBJECT_CATEGORY | STATIC_CLIFF_CATEGORY | PLAYER_CATEGORY | ENEMY_CATEGORY;
				break;
			case PLAYER_CATEGORY:
				maskBits = STATIC_OBJECT_CATEGORY | STATIC_CLIFF_CATEGORY | PLAYER_CATEGORY | ENEMY_CATEGORY | ENEMY_BULLET_CATEGORY;
				break;
			case PLAYER_BULLET_CATEGORY:
				maskBits = STATIC_OBJECT_CATEGORY | ENEMY_CATEGORY;
				break;
			case ENEMY_CATEGORY:
				maskBits = STATIC_OBJECT_CATEGORY | STATIC_CLIFF_CATEGORY | PLAYER_CATEGORY | PLAYER_BULLET_CATEGORY | ENEMY_CATEGORY;
				break;
			case ENEMY_BULLET_CATEGORY:
				maskBits = STATIC_OBJECT_CATEGORY | PLAYER_CATEGORY;
				break;
		}
	}
	Box2DFixtureDefSetMaskBits(fixtureDef, maskBits);
	Box2DFixture* fixture = Box2DBodyCreateFixtureFromFixtureDef(body, fixtureDef);
	Box2DFixtureSetSensor(fixture, isSensor);
	Box2DFixtureDefDestroy(fixtureDef);
	if (isDisk) {
		Box2DCircleShapeDestroy(shape);
	} else {
		Box2DPolygonShapeDestroy(shape);
	}
	
	if (isDynamic) {
		Box2DBodySetLinearDamping(body, linearDamping);
		Box2DBodySetAngularDamping(body, 0.0);
		Box2DBodySetFixedRotation(body, true);
		Box2DBodySetMassData(body, mass, (Vec2F) { 0.0, 0.0 }, 0.0);
	}

	return body;
}

float Box2DUtilsCheckEyeSight_RayCastCallback(Box2DFixture* fixture, Vec2F point, Vec2F normal, float fraction, void* userData) {
	(void)fixture;
	(void)point;
	(void)normal;
	(void)fraction;
	*((bool*)userData) = false;
	return 0.0f;
}

bool Box2DUtilsCheckEyeSight(Vec2F from, Vec2F to, uint16_t categoryMask) {
	if (Vec2FEquals(from, to)) {
		return true;
	}
	
	bool result = true;
	Box2DRayCastListener* rayCastListener = Box2DRayCastListenerCreate(Box2DUtilsCheckEyeSight_RayCastCallback, categoryMask, &result);
	Box2DWorldRayCast(CurrentLevel()->world, rayCastListener, from, to);
	Box2DRayCastListenerDestroy(rayCastListener);
	return result;
}
