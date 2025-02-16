#include <pinball/objects/Flipper.h>
#include <m2/Game.h>
#include <box2d/b2_polygon_shape.h>
#include <m2/box2d/Detail.h>
#include <m2/box2d/Shape.h>

namespace {
	constexpr auto MAX_FLIPPER_SWEEP_RADS = m2::to_radians(60.0f);
	constexpr auto FLIPPER_SWEEP_UP_SPEED = 16.0f;
	// Down speed is intentionally slow. Spanning the flipper should have a negative impact.
	constexpr auto FLIPPER_SWEEP_DOWN_SPEED = 4.0f;

	enum class FlipperState {
		RESTING,
		GOING_UP,
		FULLY_UP,
		GOING_DOWN
	};

	struct FlipperImpl final : m2::ObjectImpl {
		float initialRotation; // Clamped to [0, 2*PI)
		FlipperState state{FlipperState::RESTING};

		explicit FlipperImpl(const float initialRotation_) : initialRotation(m2::ClampRadiansTo2Pi(initialRotation_)) {}
	};
}

m2::void_expected LoadFlipper(m2::Object& obj, const bool rightFlipper) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(rightFlipper ? m2g::pb::SPRITE_BASIC_FLIPPER_RIGHT : m2g::pb::SPRITE_BASIC_FLIPPER_LEFT));

	obj.impl = std::make_unique<FlipperImpl>(obj.orientation);
	auto* flipper = dynamic_cast<FlipperImpl*>(obj.impl.get());

	auto& phy = obj.add_physique();
	b2BodyDef bodyDef;
	bodyDef.type = b2_kinematicBody;
	bodyDef.position.Set(obj.position.x, obj.position.y);
	bodyDef.angle = obj.orientation;
	bodyDef.linearVelocity = {};
	bodyDef.angularVelocity = 0.0f;
	bodyDef.linearDamping = 0.1f;
	bodyDef.angularDamping = 0.1f;
	bodyDef.allowSleep = true;
	bodyDef.awake = true;
	bodyDef.fixedRotation = false;
	bodyDef.bullet = true;
	bodyDef.enabled = true;
	bodyDef.userData.pointer = obj.physique_id();
	bodyDef.gravityScale = 0.0f;
	b2Body* body = M2_LEVEL.world->CreateBody(&bodyDef);
	{
		b2FixtureDef fixtureDef;
		const auto chain = m2::box2d::GenerateChainShape(sprite.Pb().regular().foreground_fixtures().chain_fixture(), sprite.Ppm());
		fixtureDef.shape = &chain;
		fixtureDef.friction = 0.1f;
		fixtureDef.restitution = 1.0f;
		fixtureDef.restitutionThreshold = 0.0f;
		fixtureDef.density = 0.0f; // Kinematic object doesn't need mass
		fixtureDef.isSensor = false;
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	}
	for (const auto& circ : sprite.Pb().regular().foreground_fixtures().circle_fixtures()) {
		b2FixtureDef fixtureDef;
		const auto circle = m2::box2d::GenerateCircleShape(circ, sprite.Ppm());
		fixtureDef.shape = &circle;
		fixtureDef.friction = 0.1f;
		fixtureDef.restitution = 1.0f;
		fixtureDef.restitutionThreshold = 0.0f;
		fixtureDef.density = 0.0f; // Kinematic object doesn't need mass
		fixtureDef.isSensor = false;
		fixtureDef.filter.categoryBits = m2::box2d::FIXTURE_CATEGORY_FRIEND_ON_FOREGROUND;
		fixtureDef.filter.maskBits = 0xFFFF; // Collide with everything
		body->CreateFixture(&fixtureDef);
	}
	phy.body = m2::box2d::BodyUniquePtr{body};

	MAYBE auto& gfx = obj.add_graphic(rightFlipper ? m2g::pb::SPRITE_BASIC_FLIPPER_RIGHT : m2g::pb::SPRITE_BASIC_FLIPPER_LEFT);

	if (rightFlipper) {
		phy.pre_step = [flipper](m2::Physique& phy_) {
			if (flipper->state == FlipperState::RESTING && M2_GAME.events.is_key_down(m2::Key::RIGHT)) {
				phy_.body->SetAngularVelocity(FLIPPER_SWEEP_UP_SPEED);
				flipper->state = FlipperState::GOING_UP;
			}
			if (flipper->state == FlipperState::FULLY_UP && not M2_GAME.events.is_key_down(m2::Key::RIGHT)) {
				phy_.body->SetAngularVelocity(-FLIPPER_SWEEP_DOWN_SPEED);
				flipper->state = FlipperState::GOING_DOWN;
			}
		};
		phy.post_step = [flipper](m2::Physique& phy_) {
			if (flipper->state == FlipperState::GOING_UP && m2::is_less(MAX_FLIPPER_SWEEP_RADS, m2::AngleAbsoluteDifference(phy_.body->GetAngle(), flipper->initialRotation), 0.001f)) {
				phy_.body->SetTransform(phy_.body->GetTransform().p, flipper->initialRotation + MAX_FLIPPER_SWEEP_RADS);
				phy_.body->SetAngularVelocity(0.0f);
				flipper->state = FlipperState::FULLY_UP;
			}
			if (flipper->state == FlipperState::GOING_DOWN && m2::is_negative(m2::AngleDifference(phy_.body->GetAngle(), flipper->initialRotation), 0.001f)) {
				phy_.body->SetTransform(phy_.body->GetTransform().p, flipper->initialRotation);
				phy_.body->SetAngularVelocity(0.0f);
				flipper->state = FlipperState::RESTING;
			}
		};
	} else {
		phy.pre_step = [flipper](m2::Physique& phy_) {
			if (flipper->state == FlipperState::RESTING && M2_GAME.events.is_key_down(m2::Key::LEFT)) {
				phy_.body->SetAngularVelocity(-FLIPPER_SWEEP_UP_SPEED);
				flipper->state = FlipperState::GOING_UP;
			}
			if (flipper->state == FlipperState::FULLY_UP && not M2_GAME.events.is_key_down(m2::Key::LEFT)) {
				phy_.body->SetAngularVelocity(FLIPPER_SWEEP_DOWN_SPEED);
				flipper->state = FlipperState::GOING_DOWN;
			}
		};
		phy.post_step = [flipper](m2::Physique& phy_) {
			if (flipper->state == FlipperState::GOING_UP && m2::is_less(MAX_FLIPPER_SWEEP_RADS, m2::AngleAbsoluteDifference(phy_.body->GetAngle(), flipper->initialRotation), 0.001f)) {
				phy_.body->SetTransform(phy_.body->GetTransform().p, flipper->initialRotation - MAX_FLIPPER_SWEEP_RADS);
				phy_.body->SetAngularVelocity(0.0f);
				flipper->state = FlipperState::FULLY_UP;
			}
			if (flipper->state == FlipperState::GOING_DOWN && m2::is_negative(m2::AngleDifference(flipper->initialRotation, phy_.body->GetAngle()), 0.001f)) {
				phy_.body->SetTransform(phy_.body->GetTransform().p, flipper->initialRotation);
				phy_.body->SetAngularVelocity(0.0f);
				flipper->state = FlipperState::RESTING;
			}
		};
	}

	return {};
}
