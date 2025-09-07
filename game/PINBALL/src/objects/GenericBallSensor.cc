#include <pinball/Objects.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <m2/Game.h>

m2::void_expected LoadGenericBallSensor(m2::Object& obj, const m2::pb::PhysicsLayer physicsLayer,
	const std::function<void(m2::Physique& sensor, m2::Physique& ball, const m2::box2d::Contact&)>& onCollisionWithBall,
	const std::function<void(m2::Physique& sensor, m2::Physique& ball)>& offCollisionWithBall) {

	const auto type = obj.GetType();
	const auto spriteType = *M2_GAME.GetMainSpriteOfObject(type);
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(spriteType));

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::DYNAMIC,
		.fixedRotation = true,
		.gravityScale = 0.0f,
		.allowSleeping = false,
		.isBullet = true
	};
	for (const auto& fixturePb : sprite.OriginalPb().regular().fixtures()) {
		rigidBodyDef.fixtures.emplace_back(m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::ToShape(fixturePb, sprite.Ppm()),
			.isSensor = true,
			.colliderFilter = m2::third_party::physics::ColliderParams{
				.belongsTo = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT,
				.collidesWith = m2::third_party::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT
			}
		});
	}
	phy.body[m2::I(physicsLayer)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef,
		obj.GetPhysiqueId(), obj.position, obj.orientation, physicsLayer);

	obj.AddGraphic(m2::ForegroundDrawLayer::F0_BOTTOM, spriteType);

	// The sensor collides with only the ball
	phy.onCollision = [onCollisionWithBall](m2::Physique& self, m2::Physique& ball, const m2::box2d::Contact& contact) {
		if (onCollisionWithBall) {
			if (ball.Owner().GetType() == m2g::pb::BALL) {
				onCollisionWithBall(self, ball, contact);
			}
		}
	};
	phy.offCollision = [offCollisionWithBall](m2::Physique& self, m2::Physique& ball) {
		if (offCollisionWithBall) {
			if (ball.Owner().GetType() == m2g::pb::BALL) {
				offCollisionWithBall(self, ball);
			}
		}
	};

	return {};
}
