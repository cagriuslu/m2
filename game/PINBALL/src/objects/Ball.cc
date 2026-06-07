#include <pinball/Objects.h>
#include <pinball/Pinball.h>
#include <m2/Game.h>
#include <box2d/b2_circle_shape.h>
#include <m2/thirdparty/physics/ColliderCategory.h>
#include <m2/Log.h>

// The ball is 0.5m in diameter.

namespace {
	struct BallImpl final : m2::HeapObjectImpl {
		std::optional<m2::VecF> lastCollidedWallPosition;

		explicit BallImpl(m2::Object&) : HeapObjectImpl() {}
	};
}

m2::void_expected LoadBall(m2::Object& obj, const m2::VecF& position) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPRITE_BASIC_BALL));

	obj.impl = std::make_unique<BallImpl>(obj);
	auto* ballImpl = dynamic_cast<BallImpl*>(std::get<std::unique_ptr<m2::HeapObjectImpl>>(obj.impl).get());

	auto& phy = obj.AddPhysique();
	phy.position = position;
	m2::thirdparty::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::thirdparty::physics::RigidBodyType::DYNAMIC,
		.fixtures = {m2::thirdparty::physics::FixtureDefinition{
			.shape = m2::thirdparty::physics::ToShape(sprite.OriginalPb().regular().fixtures(0), sprite.Ppm()),
			.friction = 0.2f,
			.restitution = 0.01f, // Control restitution via the surface
			.restitutionThresholdVelocity = 0.2f,
			.isSensor = false,
			.colliderFilter = m2::thirdparty::physics::ColliderParams{
				.belongsTo = m2::thirdparty::physics::ColliderLayer::COLLIDER_LAYER_FOREGROUND_FRIENDLY_OBJECT,
				.collidesWith = gBallGroundMask
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
	phy.body = m2::thirdparty::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), position, {});

	MAYBE auto& gfx = obj.AddGraphic(m2g::pb::UprightGraphicsLayer::UPRIGHT_GRAPHICS_DEFAULT_LAYER, m2g::pb::SPRITE_BASIC_BALL);
	gfx.position = position;

	phy.preStep = [initialPos = position](m2::Physique& phy_, const m2::Stopwatch::Duration&) {
		auto& body = std::get<m2::Physique::DynamicBody>(phy_.body);
		if (M2_GAME.events.PopKeyRelease(m2g::pb::BALL_LAUNCHER)) {
			body.ApplyForceToCenter({0.0f, -7500.0f});
			// TODO defer M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_CIRCULAR_BUMPER_SOUND], m2::AudioManager::ONCE, 0.25f);
		}
		if (M2_GAME.events.PopKeyPress(m2g::pb::RETURN)) {
			body.SetPosition(initialPos);
			M2_DEFER(CreateBallLayerSwitcher(phy_.GetOwnerId(), false));
		}
		if (M2_GAME.events.PopMouseButtonRelease(m2::MouseButton::PRIMARY)) {
			const auto mousePosition = M2_GAME.events.GetWorldPositionOfMouse();
			body.SetPosition(mousePosition);
			body.SetLinearVelocity({});
			M2_DEFER(CreateBallLayerSwitcher(phy_.GetOwnerId(), false));
		}
	};
	phy.onCollision = [ballImpl](m2::Physique& ball, const m2::Physique& other, const m2::box2d::Contact& contact) {
		if (other.GetOwner().GetType() == m2g::pb::WALLS && (not ballImpl->lastCollidedWallPosition
				|| not ballImpl->lastCollidedWallPosition->IsNear(ball.position, 0.2f))) {
			const auto velocity = std::get<m2::Physique::DynamicBody>(ball.body).GetLinearVelocity();
			// Find the speed along the collision axis. Dot product with the unit vector is the projection.
			if (const auto collisionSpeed = abs(velocity.DotProduct(contact.normal)); 5.0f < collisionSpeed) {
				const auto volume = std::clamp(collisionSpeed / 100.0f, 0.0f, 1.0f);
				M2_DEFER([volume]() {
					M2_GAME.audio_manager->Play(&M2_GAME.songs[m2g::pb::SONG_WALL_IMPACT], m2::AudioManager::ONCE, volume);
				});
			}
			ballImpl->lastCollidedWallPosition = ball.position;
		}
	};

	M2G_PROXY.ballId = obj.GetId();

	return {};
}

std::function<void()> CreateBallLayerSwitcher(const m2::ObjectId ballId, const bool toPlatform) {
	return [ballId, toPlatform]() {
		auto* object = M2_LEVEL.objects.Get(ballId);
		if (not object) {
			return;
		}
		// The ball always lives in the default physics world; only its collision mask and draw layer change so that it
		// behaves as-if it moved to the elevated platform level.
		auto& body = std::get<m2::Physique::DynamicBody>(object->GetPhysique().body);
		body.SetCollidesWith(toPlatform ? gBallPlatformMask : gBallGroundMask);
		object->MoveLayer(toPlatform
			? m2g::pb::UprightGraphicsLayer::AIRBORNE_UPRIGHT
			: m2g::pb::UprightGraphicsLayer::UPRIGHT_GRAPHICS_DEFAULT_LAYER);
	};
}
