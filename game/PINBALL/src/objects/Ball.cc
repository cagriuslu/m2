#include <pinball/Objects.h>
#include <m2/Game.h>
#include <box2d/b2_circle_shape.h>
#include <m2/third_party/physics/ColliderCategory.h>
#include <m2/Log.h>

// The ball is 0.5m in diameter.

namespace {
	struct BallImpl final : m2::ObjectImpl {
		std::optional<m2::VecF> lastCollidedWallPosition;
	};

	m2::third_party::physics::RigidBody& GetActiveRigidBody(m2::Physique& phy) {
		if (phy.body[m2::I(m2::PhysicsLayer::P0)]->IsEnabled()) {
			return *phy.body[m2::I(m2::PhysicsLayer::P0)];
		}
		return *phy.body[m2::I(m2::PhysicsLayer::P1)];
	}
}

m2::void_expected LoadBall(m2::Object& obj) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPRITE_BASIC_BALL));

	obj.impl = std::make_unique<BallImpl>();
	auto* ballImpl = dynamic_cast<BallImpl*>(obj.impl.get());

	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::DYNAMIC,
		.fixtures = {m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::ToShape(sprite.OriginalPb().regular().fixtures(0), sprite.Ppm()),
			.friction = 0.2f,
			.restitution = 0.01f, // Control restitution via the surface
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
	phy.body[m2::I(m2::PhysicsLayer::P0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::PhysicsLayer::P0);
	phy.body[m2::I(m2::PhysicsLayer::P1)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::PhysicsLayer::P1);
	phy.body[m2::I(m2::PhysicsLayer::P1)]->SetEnabled(false);

	MAYBE auto& gfx = obj.AddGraphic(m2::ForegroundDrawLayer::F0_BOTTOM, m2g::pb::SPRITE_BASIC_BALL);

	phy.preStep = [initialPos = obj.position](m2::Physique& phy_) {
		if (M2_GAME.events.PopKeyRelease(m2g::pb::BALL_LAUNCHER) /*&& M2G_PROXY.isOnBallLauncher*/) {
			if (phy_.body[m2::I(m2::PhysicsLayer::P0)]->IsEnabled()) {
				phy_.body[m2::I(m2::PhysicsLayer::P0)]->ApplyForceToCenter({0.0f, -7500.0f});
			} else if (phy_.body[m2::I(m2::PhysicsLayer::P1)]->IsEnabled()) {
				phy_.body[m2::I(m2::PhysicsLayer::P1)]->ApplyForceToCenter({0.0f, -7500.0f});
			}
			// TODO defer M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_CIRCULAR_BUMPER_SOUND], m2::AudioManager::ONCE, 0.25f);
		}
		if (M2_GAME.events.PopKeyPress(m2g::pb::RETURN)) {
			phy_.body[m2::I(m2::PhysicsLayer::P0)]->SetPosition(initialPos);
			phy_.body[m2::I(m2::PhysicsLayer::P1)]->SetPosition(initialPos);
			M2_DEFER(m2::CreateLayerMover(phy_.OwnerId(), m2::PhysicsLayer::P0, m2::ForegroundDrawLayer::F0_BOTTOM));
		}
		if (M2_GAME.events.PopMouseButtonRelease(m2::MouseButton::PRIMARY)) {
			const auto mousePosition = M2_GAME.MousePositionWorldM();
			phy_.body[m2::I(m2::PhysicsLayer::P0)]->SetPosition(mousePosition);
			phy_.body[m2::I(m2::PhysicsLayer::P0)]->SetLinearVelocity({});
			phy_.body[m2::I(m2::PhysicsLayer::P1)]->SetPosition(mousePosition);
			phy_.body[m2::I(m2::PhysicsLayer::P1)]->SetLinearVelocity({});
			M2_DEFER(m2::CreateLayerMover(phy_.OwnerId(), m2::PhysicsLayer::P0, m2::ForegroundDrawLayer::F0_BOTTOM));
		}
	};
	phy.onCollision = [&obj, ballImpl](m2::Physique& ball, const m2::Physique& other, const m2::box2d::Contact& contact) {
		if (other.Owner().GetType() == m2g::pb::WALLS && (not ballImpl->lastCollidedWallPosition
				|| not ballImpl->lastCollidedWallPosition->IsNear(obj.position, 0.2f))) {
			const auto velocity = GetActiveRigidBody(ball).GetLinearVelocity();
			// Find the speed along the collision axis. Dot product with the unit vector is the projection.
			if (const auto collisionSpeed = abs(velocity.DotProduct(contact.normal)); 5.0f < collisionSpeed) {
				const auto volume = std::clamp(collisionSpeed / 100.0f, 0.0f, 1.0f);
				M2_DEFER([volume]() {
					M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_WALL_IMPACT], m2::AudioManager::ONCE, volume);
				});
			}
			ballImpl->lastCollidedWallPosition = obj.position;
		}
	};

	M2G_PROXY.ballId = obj.GetId();

	return {};
}
