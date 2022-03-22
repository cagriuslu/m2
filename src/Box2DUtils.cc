#include "m2/Box2DUtils.hh"

#include <b2_circle_shape.h>
#include <b2_fixture.h>
#include <b2_polygon_shape.h>
#include <b2_world.h>

#include <m2/Game.hh>

b2Body* Box2DUtils_CreateBody(ID phyId, bool isDisk, bool isDynamic, m2::vec2f position, bool allowSleep, bool isBullet, bool isSensor, uint16_t categoryBits, uint16_t maskBits, m2::vec2f boxDims, m2::vec2f boxCenterOffset, float boxAngle, float diskRadius, float mass, float linearDamping, bool fixedRotation) {
	b2BodyDef bodyDef;
	if (isDynamic) {
		bodyDef.type = b2_dynamicBody;
	}
	bodyDef.position.Set(position.x, position.y);
	bodyDef.allowSleep = allowSleep;
	bodyDef.userData.pointer = phyId;
	bodyDef.bullet = isBullet;
	b2Body* body = GAME.world->CreateBody(&bodyDef);

	b2FixtureDef fixtureDef;
	b2CircleShape circle_shape;
	b2PolygonShape polygon_shape;
	if (isDisk) {
		circle_shape.m_radius = diskRadius;
		fixtureDef.shape = &circle_shape;
	} else {
		m2::vec2f halfDims = boxDims * 0.5f;
		polygon_shape.SetAsBox(halfDims.x, halfDims.y, static_cast<b2Vec2>(boxCenterOffset), boxAngle);
		fixtureDef.shape = &polygon_shape;
	}
	fixtureDef.friction = 0.05f;
	fixtureDef.filter.categoryBits = categoryBits;
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
	fixtureDef.filter.maskBits = maskBits;
	b2Fixture* fixture = body->CreateFixture(&fixtureDef);
	fixture->SetSensor(isSensor);
	
	if (isDynamic) {
		body->SetLinearDamping(linearDamping);
		body->SetAngularDamping(0.0f);
		body->SetFixedRotation(fixedRotation);
		b2MassData massData = { mass, b2Vec2{0.0f, 0.0f}, 0.0f };
		body->SetMassData(&massData);
	}

	return body;
}

float Box2DUtilsCheckEyeSight_RayCastCallback(b2Fixture* fixture, m2::vec2f point, m2::vec2f normal, float fraction, void* userData) {
	(void)fixture;
	(void)point;
	(void)normal;
	(void)fraction;
	static_cast<bool*>(userData)[0] = false;
	return 0.0f;
}

bool Box2DUtils_CheckEyeSight(m2::vec2f from, m2::vec2f to, uint16_t categoryMask) {
	if (from == to) {
		return true;
	}
	
	bool result = true;
	RayCastCallback rccb(Box2DUtilsCheckEyeSight_RayCastCallback, categoryMask, &result);
	GAME.world->RayCast(&rccb, static_cast<b2Vec2>(from), static_cast<b2Vec2>(to));
	return result;
}
