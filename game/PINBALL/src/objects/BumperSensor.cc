#include <pinball/Objects.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <m2/Game.h>
#include <m2/Log.h>

m2::void_expected LoadBumperSensor(m2::Object& obj) {
	const auto type = obj.GetType();
	const auto spriteType = *M2_GAME.GetMainSpriteOfObject(type);
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.isBullet = true
	};
	for (const auto& fixturePb : sprite.OriginalPb().regular().fixtures()) {
		rigidBodyDef.fixtures.emplace_back(m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::ToShape(fixturePb, sprite.Ppm()),
			.isSensor = false,
			.colliderFilter = m2::third_party::physics::ColliderParams{
				.belongsTo = 1,
				.collidesWith = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT
			}
		});
	}
	phy.body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::pb::PhysicsLayer::SEA_LEVEL);

	// The sensor collides with only the ball
	phy.onCollision = [](m2::Physique&, m2::Physique& ball, const m2::box2d::Contact& contact) {
		const auto contactNormal = contact.normal;
		auto* ballPhy = &ball;
		M2_DEFER(([contactNormal, ballPhy]() {
			ballPhy->body[m2::I(m2::pb::PhysicsLayer::SEA_LEVEL)]->ApplyForceToCenter(contactNormal * 5000.0f);
		}));
	};

	return {};
}
