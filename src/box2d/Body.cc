#include <m2/box2d/Body.h>
#include <m2/box2d/Detail.h>
#include <m2/Game.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

namespace m2::box2d {
	const b2BodyType body_type_map[] = {
			/* STATIC    */ b2_staticBody,
			/* KINEMATIC */ b2_kinematicBody,
			/* DYNAMIC   */ b2_dynamicBody,
	};

	const uint16_t FixtureCategory_to_collision_category_map[] = {
			/* OBSTACLE_BACKGROUND          */ FIXTURE_CATEGORY_OBSTACLE_ON_BACKGROUND,
			/* OBSTACLE                     */ FIXTURE_CATEGORY_OBSTACLE,

			/* FRIEND_ON_BACKGROUND         */ FIXTURE_CATEGORY_FRIEND_ON_BACKGROUND,
			/* FRIEND_ON_FOREGROUND         */ FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND,
			/* FRIEND_OFFENSE_ON_BACKGROUND */ FIXTURE_CATEGORY_FRIEND_OFFENSE_ON_BACKGROUND,
			/* FRIEND_OFFENSE_ON_FOREGROUND */ FIXTURE_CATEGORY_FRIEND_OFFENSE_ON_FOREGROUND,
			/* FRIEND_ITEM_ON_FOREGROUND    */ FIXTURE_CATEGORY_FRIEND_ITEM_ON_FOREGROUND,

			/* FOE_ON_BACKGROUND            */ FIXTURE_CATEGORY_FOE_ON_BACKGROUND,
			/* FOE_ON_FOREGROUND            */ FIXTURE_CATEGORY_FOE_ON_FOREGROUND,
			/* FOE_OFFENSE_ON_BACKGROUND    */ FIXTURE_CATEGORY_FOE_OFFENSE_ON_BACKGROUND,
			/* FOE_OFFENSE_ON_FOREGROUND    */ FIXTURE_CATEGORY_FOE_OFFENSE_ON_FOREGROUND,
	};

	const uint16_t FixtureCategory_to_collision_bits_map[] = {
			/* OBSTACLE_BACKGROUND          */ FIXTURE_CATEGORY_OBSTACLE_ON_BACKGROUND | FIXTURE_CATEGORY_FRIEND_ALL_BACKGROUND | FIXTURE_CATEGORY_FRIEND_ITEM_ON_FOREGROUND | FIXTURE_CATEGORY_FOE_ALL_BACKGROUND,
			/* OBSTACLE                     */ FIXTURE_CATEGORY_OBSTACLE | FIXTURE_CATEGORY_FRIEND_ALL | FIXTURE_CATEGORY_FRIEND_ITEM_ON_FOREGROUND | FIXTURE_CATEGORY_FOE_ALL,

			/* FRIEND_ON_BACKGROUND         */ FIXTURE_CATEGORY_OBSTACLE_ON_BACKGROUND | FIXTURE_CATEGORY_FOE_ALL_BACKGROUND,
			/* FRIEND_ON_FOREGROUND         */ FIXTURE_CATEGORY_OBSTACLE_ON_FOREGROUND | FIXTURE_CATEGORY_FRIEND_ITEM_ON_FOREGROUND | FIXTURE_CATEGORY_FOE_ALL_FOREGROUND,
			/* FRIEND_OFFENSE_ON_BACKGROUND */ FIXTURE_CATEGORY_FOE_ON_BACKGROUND,
			/* FRIEND_OFFENSE_ON_FOREGROUND */ FIXTURE_CATEGORY_FOE_ON_FOREGROUND,
			/* FRIEND_ITEM_ON_FOREGROUND    */ FIXTURE_CATEGORY_OBSTACLE | FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND,

			/* FOE_ON_BACKGROUND            */ FIXTURE_CATEGORY_OBSTACLE_ON_BACKGROUND | FIXTURE_CATEGORY_FRIEND_ALL_BACKGROUND | FIXTURE_CATEGORY_FOE_ON_BACKGROUND,
			/* FOE_ON_FOREGROUND            */ FIXTURE_CATEGORY_OBSTACLE_ON_FOREGROUND | FIXTURE_CATEGORY_FRIEND_ALL_FOREGROUND | FIXTURE_CATEGORY_FOE_ON_FOREGROUND,
			/* FOE_OFFENSE_ON_BACKGROUND    */ FIXTURE_CATEGORY_FRIEND_ON_BACKGROUND,
			/* FOE_OFFENSE_ON_FOREGROUND    */ FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND,
	};
}

void m2::box2d::BodyDeleter::operator()(b2Body* body) {
	if (M2_LEVEL.world->IsLocked()) {
		throw std::runtime_error("b2Body is destroyed during physics step");
	} else {
		M2_LEVEL.world->DestroyBody(body);
	}
}

m2::box2d::BodyUniquePtr m2::box2d::create_body(b2World& world, Id physique_id, m2::VecF position, const pb::BodyBlueprint& blueprint) {
	if (M2_LEVEL.world->IsLocked()) {
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

	auto fixture_creator = [&body](const pb::Fixture& fixture_blueprint) {
		b2FixtureDef fixtureDef;
		b2CircleShape circle_shape;
		b2PolygonShape polygon_shape;

		if (fixture_blueprint.has_circ()) {
			circle_shape.m_radius = fixture_blueprint.circ().radius();
			circle_shape.m_p = static_cast<b2Vec2>(VecF{fixture_blueprint.circ().center_offset()});
			fixtureDef.shape = &circle_shape;
		} else {
			m2::VecF halfDims = VecF{fixture_blueprint.rect().dims()} * 0.5f;
			polygon_shape.SetAsBox(halfDims.x, halfDims.y, static_cast<b2Vec2>(VecF{fixture_blueprint.rect().center_offset()}), fixture_blueprint.rect().angle());
			fixtureDef.shape = &polygon_shape;
		}
		fixtureDef.friction = fixture_blueprint.has_friction() ? fixture_blueprint.friction() : 0.1f;
		fixtureDef.filter.categoryBits = FixtureCategory_to_collision_category_map[fixture_blueprint.category()];
		fixtureDef.filter.maskBits = FixtureCategory_to_collision_bits_map[fixture_blueprint.category()];
		b2Fixture* fixture = body->CreateFixture(&fixtureDef);
		fixture->SetSensor(fixture_blueprint.is_sensor());
	};
	if (blueprint.has_background_fixture()) {
		fixture_creator(blueprint.background_fixture());
	}
	if (blueprint.has_foreground_fixture()) {
		fixture_creator(blueprint.foreground_fixture());
	}

	if (blueprint.type() == pb::BodyType::DYNAMIC) {
		body->SetLinearDamping(blueprint.linear_damping());
		body->SetAngularDamping(0.0f);
		body->SetFixedRotation(blueprint.fixed_rotation());
		b2MassData massData = {blueprint.mass(), b2Vec2{0.0f, 0.0f}, 0.0f};
		body->SetMassData(&massData);

		auto gravity_scale = blueprint.gravity_scale();
		if (gravity_scale < 0.0f) {
			body->SetGravityScale(0.0f);
		} else {
			body->SetGravityScale(gravity_scale == 0.0f ? 1.0f : gravity_scale);
		}
	}

	return BodyUniquePtr{body};
}

bool m2::box2d::has_obstacle(const b2Body* body) {
	bool is_obstacle = false;
	for (const auto* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
		auto category_bits = fixture->GetFilterData().categoryBits;
		is_obstacle |= category_bits & FIXTURE_CATEGORY_OBSTACLE;
	}
	return is_obstacle;
}
