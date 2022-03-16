#include "m2/Box2DUtils.h"
#include "m2/Game.h"

Box2DBody* Box2DUtils_CreateBody(ID phyId, bool isDisk, bool isDynamic, Vec2F position, bool allowSleep, bool isBullet, bool isSensor, uint16_t categoryBits, uint16_t maskBits, Vec2F boxDims, Vec2F boxCenterOffset, float boxAngle, float diskRadius, float mass, float linearDamping, bool fixedRotation) {
	Box2DBodyDef* bodyDef = Box2DBodyDefCreate();
	if (isDynamic) {
		Box2DBodyDefSetTypeDynamic(bodyDef);
	}
	Box2DBodyDefSetPosition(bodyDef, position);
	Box2DBodyDefSetAllowSleep(bodyDef, allowSleep);
	Box2DBodyDefSetUserData(bodyDef, (void*) ((uintptr_t) phyId));
	Box2DBodyDefSetBullet(bodyDef, isBullet);
	Box2DBody* body = Box2DWorldCreateBody(GAME->world, bodyDef);
	Box2DBodyDefDestroy(bodyDef);

	Box2DShape* shape = NULL;
	if (isDisk) {
		Box2DCircleShape* circleShape = Box2DCircleShapeCreate();
		Box2DCircleShapeSetRadius(circleShape, diskRadius);
		shape = circleShape;
	} else {
		Box2DPolygonShape* boxShape = Box2DPolygonShapeCreate();
		Box2DPolygonShapeSetAsBoxEx(boxShape, Vec2F_Mul(boxDims, 0.5f), boxCenterOffset, boxAngle); // convert to half dims
		shape = boxShape;
	}
	Box2DFixtureDef* fixtureDef = Box2DFixtureDefCreate();
	Box2DFixtureDefSetShape(fixtureDef, shape);
	Box2DFixtureDefSetFriction(fixtureDef, 0.05f);
	Box2DFixtureDefSetCategoryBits(fixtureDef, categoryBits);
	// If mask is not provided, infer it
	if (maskBits == 0) {
		switch (categoryBits) {
			case CATEGORY_STATIC_OBJECT:
				maskBits = CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF | CATEGORY_PLAYER | CATEGORY_PLAYER_BULLET | CATEGORY_ENEMY | CATEGORY_ENEMY_BULLET;
				break;
			case CATEGORY_STATIC_CLIFF:
				maskBits = CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF | CATEGORY_PLAYER | CATEGORY_ENEMY;
				break;
			case CATEGORY_PLAYER:
				maskBits = CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF | CATEGORY_PLAYER | CATEGORY_ENEMY | CATEGORY_ENEMY_BULLET | CATEGORY_ENEMY_MELEE_WEAPON;
				break;
			case CATEGORY_PLAYER_BULLET:
				maskBits = CATEGORY_STATIC_OBJECT | CATEGORY_ENEMY;
				break;
			case CATEGORY_PLAYER_MELEE_WEAPON:
				maskBits = CATEGORY_ENEMY;
				break;
			case CATEGORY_ENEMY:
				maskBits = CATEGORY_STATIC_OBJECT | CATEGORY_STATIC_CLIFF | CATEGORY_PLAYER | CATEGORY_PLAYER_BULLET | CATEGORY_PLAYER_MELEE_WEAPON | CATEGORY_ENEMY;
				break;
			case CATEGORY_ENEMY_BULLET:
				maskBits = CATEGORY_STATIC_OBJECT | CATEGORY_PLAYER;
				break;
			case CATEGORY_ENEMY_MELEE_WEAPON:
				maskBits = CATEGORY_PLAYER;
				break;
			default:
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
		Box2DBodySetAngularDamping(body, 0.0f);
		Box2DBodySetFixedRotation(body, fixedRotation);
		Box2DBodySetMassData(body, mass, (Vec2F) { 0.0f, 0.0f }, 0.0f);
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

bool Box2DUtils_CheckEyeSight(Vec2F from, Vec2F to, uint16_t categoryMask) {
	if (Vec2F_Equals(from, to)) {
		return true;
	}
	
	bool result = true;
	Box2DRayCastListener* rayCastListener = Box2DRayCastListenerCreate(Box2DUtilsCheckEyeSight_RayCastCallback, categoryMask, &result);
	Box2DWorldRayCast(GAME->world, rayCastListener, from, to);
	Box2DRayCastListenerDestroy(rayCastListener);
	return result;
}
