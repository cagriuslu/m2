#include <pinball/Objects.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <m2/Game.h>

m2::void_expected LoadCircularBumperSensor(m2::Object& obj) {
	const auto type = obj.GetType();
	const auto spriteType = *M2_GAME.GetMainSpriteOfObject(type);
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.isBullet = false
	};
	for (const auto& fixturePb : sprite.OriginalPb().regular().fixtures()) {
		rigidBodyDef.fixtures.emplace_back(m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::ToShape(fixturePb, sprite.Ppm()),
			.isSensor = false,
			.colliderFilter = m2::third_party::physics::ColliderParams{
				.belongsTo = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT,
				.collidesWith = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT
			}
		});
	}
	phy.body[I(m2::PhysicsLayer::P0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::PhysicsLayer::P0);

	obj.AddGraphic(m2::ForegroundDrawLayer::F0_BOTTOM, spriteType);

	// The sensor collides with only the ball
	phy.onCollision = [](m2::Physique&, m2::Physique& ball, const m2::box2d::Contact& contact) {
		const auto contactNormal = contact.normal;
		auto* ballPhy = &ball;
		M2_DEFER(([contactNormal, ballPhy]() {
			ballPhy->body[I(m2::PhysicsLayer::P0)]->ApplyForceToCenter(contactNormal * 6000.0f);
			M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_CIRCULAR_BUMPER_SOUND], m2::AudioManager::ONCE, 0.25f);
		}));
	};

	return {};
}
