#include <rpg/Objects.h>
#include <m2/Game.h>
#include <m2/game/Timer.h>

struct Spikes : public m2::ObjectImpl {
	std::optional<m2::Timer> trigger_timer;
};

m2::void_expected rpg::create_spikes(m2::Object& obj) {
	const auto& spikes_in = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPIKES_IN));
	const auto& spikes_out = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(m2g::pb::SPIKES_OUT));

	// Create physique component
	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::STATIC);
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(spikes_in.BackgroundColliderCircRadiusM());
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);
	bp.mutable_background_fixture()->set_is_sensor(true);
	phy.body = m2::box2d::CreateBody(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);

	// Create graphic component
	auto& gfx = obj.add_graphic(m2g::pb::SPIKES_IN);

	// Create custom data
	obj.impl = std::make_unique<Spikes>();
	auto& impl = dynamic_cast<Spikes&>(*obj.impl);

	phy.preStep = [&, bp](MAYBE m2::Physique& self) {
		// Check if the spikes are in, and triggered
		if (std::get<const m2::Sprite*>(gfx.visual) == &spikes_in && impl.trigger_timer) {
			if (impl.trigger_timer->has_ticks_passed(200)) {
				std::get<const m2::Sprite*>(gfx.visual) = &spikes_out;
				impl.trigger_timer = m2::Timer{};
				// Recreate the body so that collision is reset, otherwise the Player standing on the spikes doesn't collide again
				self.body = m2::box2d::CreateBody(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);
			}
		} else if (std::get<const m2::Sprite*>(gfx.visual) == &spikes_out && impl.trigger_timer) {
			// Spikes are out and triggered
			if (impl.trigger_timer->has_ticks_passed(1000)) {
				std::get<const m2::Sprite*>(gfx.visual) = &spikes_in;
				impl.trigger_timer.reset();
				// Recreate the body so that collision is reset, otherwise the Player standing on the spikes doesn't collide again
				self.body = m2::box2d::CreateBody(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);
			}
		}
	};
	phy.onCollision = [&spikes_in, &spikes_out, &impl, &gfx](MAYBE m2::Physique& self, MAYBE m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		// Check if the spikes are in, and not triggered
		if (std::get<const m2::Sprite*>(gfx.visual) == &spikes_in && not impl.trigger_timer) {
			impl.trigger_timer = m2::Timer{};
		} else if (std::get<const m2::Sprite*>(gfx.visual) == &spikes_out) {
			// Spikes are out
			if (auto* other_char = other.owner().get_character(); other_char){
				m2g::pb::InteractionData data;
				data.set_hit_damage(1.0f);
				other_char->ExecuteInteraction(data);
			}
		}
	};

	return {};
}
