#include <pinball/objects/TriangleTrap.h>
#include <m2/Game.h>
#include <m2/third_party/physics/ColliderCategory.h>

m2::void_expected LoadTriangleTrap(m2::Object& obj) {
	const auto type = obj.GetType();
	const auto spriteType = *M2_GAME.GetMainSpriteOfObject(type);
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::DYNAMIC,
		.fixtures = {m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::CircleShape{.radius = 0.55f}, // TODO try triangle using polygon
			.friction = 0.2f,
			.restitution = 0.65f,
			.restitutionThresholdVelocity = 0.2f,
			.isSensor = false,
			.colliderFilter = m2::third_party::physics::ColliderParams{
				.belongsTo = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT,
				.collidesWith = 0xFFFF
			}
		}},
		.linearDamping = 0.0f,
		.fixedRotation = false,
		.mass = 0.1f,
		.inertia = 0.1f,
		.allowSleeping = false,
		.initiallyAwake = true,
		.isBullet = true,
		.initiallyEnabled = true
	};
	phy.body[I(m2::ForegroundLayer::F0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation);

	MAYBE auto& gfx = obj.AddGraphic(spriteType);

	return {};
}
