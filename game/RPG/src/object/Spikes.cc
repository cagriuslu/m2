#include <rpg/Objects.h>
#include <m2/Game.h>
#include <m2/game/Timer.h>
#include <m2/third_party/physics/ColliderCategory.h>

struct Spikes : public m2::ObjectImpl {
	std::optional<m2::Timer> trigger_timer;
};

m2::void_expected rpg::create_spikes(m2::Object& obj) {
	const auto& spikes_in = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPIKES_IN));
	const auto& spikes_out = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPIKES_OUT));

	// Create physique component
	auto& phy = obj.AddPhysique();
	m2::third_party::physics::RigidBodyDefinition rigidBodyDef{
		.bodyType = m2::third_party::physics::RigidBodyType::STATIC,
		.fixtures = {m2::third_party::physics::FixtureDefinition{
			.shape = m2::third_party::physics::CircleShape::FromSpriteCircleFixture(spikes_in.OriginalPb().regular().fixtures(0).circle(), spikes_in.Ppm()),
			.isSensor = true,
			.colliderFilter = m2::third_party::physics::gColliderCategoryToParams[m2::I(m2::third_party::physics::ColliderCategory::COLLIDER_CATEGORY_BACKGROUND_OBSTACLE)]
		}},
		.allowSleeping = true,
		.initiallyAwake = false,
		.isBullet = false
	};
	phy.body[I(m2::PhysicsLayer::P0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::PhysicsLayer::P0);

	// Create graphic component
	auto& gfx = obj.AddGraphic(m2::ForegroundDrawLayer::F0_BOTTOM, m2g::pb::SPIKES_IN);

	// Create custom data
	obj.impl = std::make_unique<Spikes>();
	auto& impl = dynamic_cast<Spikes&>(*obj.impl);

	phy.preStep = [&, rigidBodyDef](MAYBE m2::Physique& self, const m2::Stopwatch::Duration&) {
		// Check if the spikes are in, and triggered
		if (std::get<const m2::Sprite*>(gfx.visual) == &spikes_in && impl.trigger_timer) {
			if (impl.trigger_timer->has_ticks_passed(200)) {
				std::get<const m2::Sprite*>(gfx.visual) = &spikes_out;
				impl.trigger_timer = m2::Timer{};
				// Recreate the body so that collision is reset, otherwise the Player standing on the spikes doesn't collide again
				phy.body[I(m2::PhysicsLayer::P0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::PhysicsLayer::P0);
			}
		} else if (std::get<const m2::Sprite*>(gfx.visual) == &spikes_out && impl.trigger_timer) {
			// Spikes are out and triggered
			if (impl.trigger_timer->has_ticks_passed(1000)) {
				std::get<const m2::Sprite*>(gfx.visual) = &spikes_in;
				impl.trigger_timer.reset();
				// Recreate the body so that collision is reset, otherwise the Player standing on the spikes doesn't collide again
				phy.body[I(m2::PhysicsLayer::P0)] = m2::third_party::physics::RigidBody::CreateFromDefinition(rigidBodyDef, obj.GetPhysiqueId(), obj.position, obj.orientation, m2::PhysicsLayer::P0);
			}
		}
	};
	phy.onCollision = [&spikes_in, &spikes_out, &impl, &gfx](MAYBE m2::Physique& self, MAYBE m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		// Check if the spikes are in, and not triggered
		if (std::get<const m2::Sprite*>(gfx.visual) == &spikes_in && not impl.trigger_timer) {
			impl.trigger_timer = m2::Timer{};
		} else if (std::get<const m2::Sprite*>(gfx.visual) == &spikes_out) {
			// Spikes are out
			if (auto* other_char = other.Owner().TryGetCharacter(); other_char){
				m2g::pb::InteractionData data;
				data.set_hit_damage(1.0f);
				other_char->ExecuteInteraction(data);
			}
		}
	};

	return {};
}
