#include <m2/box2d/Utils.h>
#include <b2_circle_shape.h>
#include <b2_fixture.h>
#include <b2_polygon_shape.h>
#include <b2_world.h>
#include <exception>

namespace m2::box2d {
	const b2BodyType body_type_map[] = {
		/* STATIC    */ b2_staticBody,
		/* KINEMATIC */ b2_kinematicBody,
		/* DYNAMIC   */ b2_dynamicBody,
	};

	const uint16_t collision_category_map[] = {
		/* OBSTACLE              */ BODY_CATEGORY_OBSTACLE,
		/* OBSTACLE_BACKGROUND   */ BODY_CATEGORY_OBSTACLE_BACKGROUND,
		/* FRIEND                */ BODY_CATEGORY_FRIEND,
		/* FRIEND_FOREGROUND_OBJ */ BODY_CATEGORY_FRIEND_FOREGROUND,
		/* FRIEND_BACKGROUND_AOE */ BODY_CATEGORY_FRIEND_BACKGROUND,
		/* FRIEND_PICKUP         */ BODY_CATEGORY_FRIEND_PICKUP,
		/* FOE                   */ BODY_CATEGORY_FOE,
		/* FOE_FOREGROUND_OBJ    */ BODY_CATEGORY_FOE_FOREGROUND,
		/* FOE_BACKGROUND_AOE    */ BODY_CATEGORY_FOE_BACKGROUND,
	};

	const uint16_t collision_mask_map[] = {
		/* OBSTACLE              */ BODY_CATEGORY_OBSTACLE            | BODY_CATEGORY_FRIEND            | BODY_CATEGORY_FOE            | BODY_CATEGORY_FRIEND_PICKUP,
		/* OBSTACLE_BACKGROUND   */ BODY_CATEGORY_OBSTACLE_BACKGROUND | BODY_CATEGORY_FRIEND_BACKGROUND | BODY_CATEGORY_FOE_BACKGROUND | BODY_CATEGORY_FRIEND_PICKUP,
		/* FRIEND                */ BODY_CATEGORY_OBSTACLE                                              | BODY_CATEGORY_FOE            | BODY_CATEGORY_FRIEND_PICKUP,
		/* FRIEND_FOREGROUND_OBJ */ BODY_CATEGORY_OBSTACLE_FOREGROUND                                   | BODY_CATEGORY_FOE_BACKGROUND                              ,
		/* FRIEND_BACKGROUND_AOE */                                                                       BODY_CATEGORY_FOE_BACKGROUND                              ,
		/* FRIEND_PICKUP         */ BODY_CATEGORY_OBSTACLE            | BODY_CATEGORY_FRIEND                                                                        ,
		/* FOE                   */ BODY_CATEGORY_OBSTACLE            | BODY_CATEGORY_FRIEND            | BODY_CATEGORY_FOE                                         ,
		/* FOE_FOREGROUND_OBJ    */ BODY_CATEGORY_OBSTACLE_FOREGROUND | BODY_CATEGORY_FRIEND_BACKGROUND                                                             ,
		/* FOE_BACKGROUND_AOE    */                                     BODY_CATEGORY_FRIEND_BACKGROUND                                                             ,
	};
}

b2Body* m2::box2d::create_body(b2World& world, Id physique_id, m2::Vec2f position, const pb::BodyBlueprint& blueprint) {
	if (GAME.is_phy_stepping) {
		throw M2ERROR("b2Body is created during physics step");
	}

	b2BodyDef bodyDef;
	bodyDef.type = body_type_map[blueprint.type()];
	bodyDef.position.Set(position.x, position.y);
	bodyDef.allowSleep = blueprint.allow_sleep();
	bodyDef.userData.pointer = physique_id;
	bodyDef.bullet = blueprint.is_bullet();
	b2Body* body = world.CreateBody(&bodyDef);
	if (!body) {
		// Most probably this function is called during physics step
		throw M2ERROR("CreateBody returned null");
	}

	b2FixtureDef fixtureDef;
	b2CircleShape circle_shape;
	b2PolygonShape polygon_shape;
	if (blueprint.has_circ()) {
		circle_shape.m_radius = blueprint.circ().radius();
		fixtureDef.shape = &circle_shape;
	} else {
		m2::Vec2f halfDims = Vec2f{blueprint.rect().dims()} * 0.5f;
		polygon_shape.SetAsBox(halfDims.x, halfDims.y, static_cast<b2Vec2>(Vec2f{blueprint.rect().center_offset()}), blueprint.rect().angle());
		fixtureDef.shape = &polygon_shape;
	}
	fixtureDef.friction = 0.05f;
	fixtureDef.filter.categoryBits = collision_category_map[blueprint.category()];
	fixtureDef.filter.maskBits = collision_mask_map[blueprint.category()];
	b2Fixture* fixture = body->CreateFixture(&fixtureDef);
	fixture->SetSensor(blueprint.is_sensor());

	if (blueprint.type() == pb::BodyType::DYNAMIC) {
		body->SetLinearDamping(blueprint.linear_damping());
		body->SetAngularDamping(0.0f);
		body->SetFixedRotation(blueprint.fixed_rotation());
		b2MassData massData = {blueprint.mass(), b2Vec2{0.0f, 0.0f}, 0.0f};
		body->SetMassData(&massData);
	}

	return body;
}

void m2::box2d::destroy_body(b2Body*& body) {
	if (GAME.is_phy_stepping) {
		throw std::runtime_error("b2Body is destroyed during physics step");
	} else {
		GAME.world->DestroyBody(body);
		body = nullptr;
	}
}

b2AABB m2::box2d::expand_aabb(const b2AABB& in, float amount) {
    return b2AABB{
		.lowerBound = b2Vec2{in.lowerBound.x - amount, in.lowerBound.y - amount},
		.upperBound = b2Vec2{in.upperBound.x + amount, in.upperBound.y + amount}
    };
}

bool m2::box2d::is_obstacle(const b2Fixture* fixture) {
	auto category_bits = fixture->GetFilterData().categoryBits;
	return category_bits & BODY_CATEGORY_OBSTACLE;
}
