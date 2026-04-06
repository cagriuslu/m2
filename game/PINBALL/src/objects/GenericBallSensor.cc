#include <pinball/Objects.h>
#include <m2/thirdparty/physics/ColliderCategory.h>
#include <m2/Game.h>

m2::void_expected LoadGenericBallSensor(m2::Object& obj, const m2::VecF& position, const m2::pb::PhysicsLayer physicsLayer,
	const std::function<void(m2::Physique& sensor, m2::Physique& ball, const m2::box2d::Contact&)>& onCollisionWithBall,
	const std::function<void(m2::Physique& sensor, m2::Physique& ball)>& offCollisionWithBall) {

	const auto type = obj.GetType();
	const auto spriteType = *M2_GAME.GetMainSpriteOfObject(type);
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));

	auto& phy = obj.AddPhysique();
	phy.position = position;
	m2::thirdparty::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::thirdparty::physics::RigidBodyType::DYNAMIC,
		.fixedRotation = true,
		.gravityScale = 0.0f,
		.allowSleeping = false,
		.isBullet = true
	};
	for (const auto& fixturePb : sprite.OriginalPb().regular().fixtures()) {
		rigidBodyDef.fixtures.emplace_back(m2::thirdparty::physics::FixtureDefinition{
			.shape = m2::thirdparty::physics::ToShape(fixturePb, sprite.Ppm()),
			.isSensor = true,
			.colliderFilter = m2::thirdparty::physics::ColliderParams{
				.belongsTo = m2::thirdparty::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT,
				.collidesWith = m2::thirdparty::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT
			}
		});
	}
	phy.body[m2::I(physicsLayer)] = m2::thirdparty::physics::RigidBody::CreateFromDefinition(rigidBodyDef,
		obj.GetPhysiqueId(), position, {}, physicsLayer);

	obj.AddGraphic(m2::pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT, spriteType);

	// The sensor collides with only the ball
	phy.onCollision = [onCollisionWithBall](m2::Physique& self, m2::Physique& ball, const m2::box2d::Contact& contact) {
		if (onCollisionWithBall) {
			if (ball.GetOwner().GetType() == m2g::pb::BALL) {
				onCollisionWithBall(self, ball, contact);
			}
		}
	};
	phy.offCollision = [offCollisionWithBall](m2::Physique& self, m2::Physique& ball) {
		if (offCollisionWithBall) {
			if (ball.GetOwner().GetType() == m2g::pb::BALL) {
				offCollisionWithBall(self, ball);
			}
		}
	};

	return {};
}
