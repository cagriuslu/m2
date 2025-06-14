#include <pinball/Objects.h>
#include <m2/Game.h>
#include <m2/box2d/Shape.h>
#include <m2/third_party/physics/ColliderCategory.h>

namespace {
	constexpr auto MAX_FLIPPER_SWEEP_RADS = m2::ToRadians(55.0f);
	constexpr auto FLIPPER_SWEEP_UP_SPEED = 25.0f;
	// Down speed is intentionally slow. Spamming the flipper should have a negative impact.
	constexpr auto FLIPPER_SWEEP_DOWN_SPEED = 3.0f;

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
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(rightFlipper
			? m2g::pb::SPRITE_BASIC_FLIPPER_RIGHT : m2g::pb::SPRITE_BASIC_FLIPPER_LEFT));

	obj.impl = std::make_unique<FlipperImpl>(obj.orientation);
	auto* flipper = dynamic_cast<FlipperImpl*>(obj.impl.get());

	auto& phy = obj.AddPhysique();
	const m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::KINEMATIC,
		.fixtures = m2::ToVector(
				sprite.OriginalPb().regular().fixtures() | std::views::transform([&sprite](const auto& fixture) {
					m2::third_party::physics::FixtureDefinition fixtureDef;
					fixtureDef.shape = m2::third_party::physics::ToShape(fixture, sprite.Ppm());
					fixtureDef.friction = 0.6f;
					fixtureDef.restitution = 0.60f;
					fixtureDef.restitutionThresholdVelocity = 0.0f;
					fixtureDef.isSensor = false;
					fixtureDef.colliderFilter.belongsTo =
							m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT;
					fixtureDef.colliderFilter.collidesWith = 0xFFFF;
					return fixtureDef;
				})),
		.allowSleeping = false,
		.initiallyAwake = true,
		.isBullet = true,
		.initiallyEnabled = true
	};
	phy.body[I(m2::PhysicsLayer::P0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::PhysicsLayer::P0);

	MAYBE auto& gfx = obj.AddGraphic(m2::ForegroundDrawLayer::F0_BOTTOM, rightFlipper ? m2g::pb::SPRITE_BASIC_FLIPPER_RIGHT : m2g::pb::SPRITE_BASIC_FLIPPER_LEFT);

	if (rightFlipper) {
		phy.preStep = [flipper](m2::Physique& phy_) {
			if (flipper->state == FlipperState::RESTING && M2_GAME.events.IsKeyDown(m2g::pb::RIGHT_FLIPPER)) {
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngularVelocity(FLIPPER_SWEEP_UP_SPEED);
				flipper->state = FlipperState::GOING_UP;
				// Nudge the ball ever so sligthly to the opposite direction
				M2_LEVEL.objects[M2G_PROXY.ballId].GetPhysique().body[I(m2::PhysicsLayer::P0)]->ApplyForceToCenter({-40.0f, 0.0f});
				M2_DEFER([]() {
					M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_FLIPPER_FLIP_UP_SOUND], m2::AudioManager::ONCE, 0.15f);
				});
			}
			if (flipper->state == FlipperState::FULLY_UP && not M2_GAME.events.IsKeyDown(m2g::pb::RIGHT_FLIPPER)) {
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngularVelocity(-FLIPPER_SWEEP_DOWN_SPEED);
				flipper->state = FlipperState::GOING_DOWN;
			}
		};
		// Lock to up or down position
		phy.postStep = [flipper](m2::Physique& phy_) {
			if (flipper->state == FlipperState::GOING_UP && m2::IsLess(MAX_FLIPPER_SWEEP_RADS, m2::AngleAbsoluteDifference(phy_.body[I(m2::PhysicsLayer::P0)]->GetAngle(), flipper->initialRotation), 0.001f)) {
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngle(flipper->initialRotation + MAX_FLIPPER_SWEEP_RADS);
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngularVelocity(0.0f);
				flipper->state = FlipperState::FULLY_UP;
			}
			if (flipper->state == FlipperState::GOING_DOWN && m2::IsNegative(m2::AngleDifference(phy_.body[I(m2::PhysicsLayer::P0)]->GetAngle(), flipper->initialRotation), 0.001f)) {
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngle(flipper->initialRotation);
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngularVelocity(0.0f);
				flipper->state = FlipperState::RESTING;
			}
		};
	} else {
		phy.preStep = [flipper](m2::Physique& phy_) {
			if (flipper->state == FlipperState::RESTING && M2_GAME.events.IsKeyDown(m2g::pb::LEFT_FLIPPER)) {
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngularVelocity(-FLIPPER_SWEEP_UP_SPEED);
				flipper->state = FlipperState::GOING_UP;
				// Nudge the ball ever so sligthly to the opposite direction
				M2_LEVEL.objects[M2G_PROXY.ballId].GetPhysique().body[I(m2::PhysicsLayer::P0)]->ApplyForceToCenter({40.0f, 0.0f});
				M2_DEFER([]() {
					M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_FLIPPER_FLIP_UP_SOUND], m2::AudioManager::ONCE, 0.15f);
				});
			}
			if (flipper->state == FlipperState::FULLY_UP && not M2_GAME.events.IsKeyDown(m2g::pb::LEFT_FLIPPER)) {
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngularVelocity(FLIPPER_SWEEP_DOWN_SPEED);
				flipper->state = FlipperState::GOING_DOWN;
			}
		};
		// Lock to up or down position
		phy.postStep = [flipper](m2::Physique& phy_) {
			if (flipper->state == FlipperState::GOING_UP && m2::IsLess(MAX_FLIPPER_SWEEP_RADS, m2::AngleAbsoluteDifference(phy_.body[I(m2::PhysicsLayer::P0)]->GetAngle(), flipper->initialRotation), 0.001f)) {
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngle(flipper->initialRotation - MAX_FLIPPER_SWEEP_RADS);
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngularVelocity(0.0f);
				flipper->state = FlipperState::FULLY_UP;
			}
			if (flipper->state == FlipperState::GOING_DOWN && m2::IsNegative(m2::AngleDifference(flipper->initialRotation, phy_.body[I(m2::PhysicsLayer::P0)]->GetAngle()), 0.001f)) {
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngle(flipper->initialRotation);
				phy_.body[I(m2::PhysicsLayer::P0)]->SetAngularVelocity(0.0f);
				flipper->state = FlipperState::RESTING;
			}
		};
	}

	return {};
}
