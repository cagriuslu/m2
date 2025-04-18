#include <pinball/objects/Ball.h>
#include <m2/Game.h>
#include <box2d/b2_circle_shape.h>
#include <m2/third_party/physics/ColliderCategory.h>

m2::void_expected LoadBall(m2::Object& obj) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPRITE_BASIC_BALL));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::DYNAMIC,
		.fixtures = {m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::ToShape(sprite.OriginalPb().regular().fixtures(0), sprite.Ppm()),
			.friction = 0.0f,
			.restitution = 1.0f,
			.restitutionThresholdVelocity = 0.0f,
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
	phy.body = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation);

	MAYBE auto& gfx = obj.AddGraphic(m2g::pb::SPRITE_BASIC_BALL);

	return {};
}
