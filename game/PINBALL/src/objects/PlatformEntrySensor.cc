#include <pinball/objects/Sensor.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <m2/Game.h>
#include <m2/Log.h>

m2::void_expected LoadPlatformEntrySensor(m2::Object& obj) {
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
			.isSensor = true,
			.colliderFilter = m2::third_party::physics::ColliderParams{
				.belongsTo = 1,
				.collidesWith = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT
			}
		});
	}
	phy.body[I(m2::ForegroundLayer::F0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation);

	// The sensor collides with only the ball
	phy.onCollision = [](m2::Physique&, const m2::Physique& ball, const m2::box2d::Contact&) {
		// Move the ball up to the platform
		LOG_INFO("Moving to platform level");
		M2_DEFER(m2::CreateForegroundLayerMover(ball.OwnerId(), m2::ForegroundLayer::F1));
	};

	return {};
}
