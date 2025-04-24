#include <pinball/objects/BallLauncherSensor.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <m2/Game.h>

m2::void_expected LoadBallLauncherSensor(m2::Object& obj) {
	const auto type = obj.GetType();
	const auto spriteType = *M2_GAME.GetMainSpriteOfObject(type);
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.fixtures = {m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::ToShape(sprite.OriginalPb().regular().fixtures(0), sprite.Ppm()),
			.isSensor = true,
			.colliderFilter = m2::third_party::physics::ColliderParams{
				.belongsTo = 1,
				.collidesWith = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT
			}
		}},
		.allowSleeping = true,
		.initiallyAwake = true,
		.isBullet = false,
		.initiallyEnabled = true
	};
	phy.body = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation);

	phy.onCollision = [](m2::Physique& self, m2::Physique& other, const m2::box2d::Contact&) {
		LOG_INFO("Colliding");
		M2G_PROXY.isOnBallLauncher = true;
	};
	phy.offCollision = [](m2::Physique& self, m2::Physique& other) {
		LOG_INFO("Not Colliding");
		M2G_PROXY.isOnBallLauncher = false;
	};

	return {};
}
