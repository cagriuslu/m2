#include <pinball/objects/Ball.h>
#include <m2/Game.h>
#include <box2d/b2_circle_shape.h>
#include <m2/third_party/physics/ColliderCategory.h>

// The ball is 0.5m in diameter.

m2::void_expected LoadBall(m2::Object& obj) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPRITE_BASIC_BALL));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::DYNAMIC,
		.fixtures = {m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::ToShape(sprite.OriginalPb().regular().fixtures(0), sprite.Ppm()),
			.friction = 0.2f,
			.restitution = 0.75f,
			.restitutionThresholdVelocity = 0.2f,
			.isSensor = false,
			.colliderFilter = m2::third_party::physics::ColliderParams{
				.belongsTo = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT,
				.collidesWith = 0xFFFF
			}
		}},
		.linearDamping = 0.0f,
		.fixedRotation = true,
		.mass = 1.0f,
		.inertia = 1.0f,
		.allowSleeping = false,
		.initiallyAwake = true,
		.isBullet = true,
		.initiallyEnabled = true
	};
	phy.body[I(m2::ForegroundLayer::F0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation);
	phy.body[I(m2::ForegroundLayer::F1)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::ForegroundLayer::F1);
	phy.body[I(m2::ForegroundLayer::F1)]->SetEnabled(false);

	MAYBE auto& gfx = obj.AddGraphic(m2g::pb::SPRITE_BASIC_BALL);

	phy.preStep = [](m2::Physique& phy_) {
		if (M2_GAME.events.PopKeyRelease(m2g::pb::BALL_LAUNCHER) && M2G_PROXY.isOnBallLauncher) {
			phy_.body[I(m2::ForegroundLayer::F0)]->ApplyForceToCenter({0.0f, -6000.0f});
		}
	};

	return {};
}
