#include <rpg/Objects.h>
#include <m2/Game.h>

struct Spikes : public m2::ObjectImpl {
	std::optional<m2::sdl::ticks_t> trigger_ticks;
};

m2::void_expected rpg::create_spikes(m2::Object& obj) {
	const auto& spikes_in = GAME.get_sprite(m2g::pb::SPIKES_IN);
	const auto& spikes_out = GAME.get_sprite(m2g::pb::SPIKES_OUT);

	// Create physique component
	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::STATIC);
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(spikes_in.background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::OBSTACLE_BACKGROUND);
	bp.mutable_background_fixture()->set_is_sensor(true);
	phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);

	// Create graphic component
	auto& gfx = obj.add_graphic(spikes_in);

	// Create custom data
	obj.impl = std::make_unique<Spikes>();
	auto& impl = dynamic_cast<Spikes&>(*obj.impl);

	phy.on_collision = [&](MAYBE m2::Physique& self, MAYBE m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		// Check if the spikes are in, and not triggered
		if (gfx.sprite == &spikes_in && not impl.trigger_ticks) {
			impl.trigger_ticks = m2::sdl::get_ticks() - GAME.pause_ticks;
		} else if (gfx.sprite == &spikes_out) {
			// Spikes are out
			// TODO hurt
		}
	};
	phy.post_step = [&](MAYBE m2::Physique& self) {
		// Check if the spikes are in, and triggered
		if (gfx.sprite == &spikes_in && impl.trigger_ticks) {
			auto ticks_since = m2::sdl::get_ticks_since(*impl.trigger_ticks, GAME.pause_ticks);
			if (200 < ticks_since) {
				gfx.sprite = &spikes_out;
				impl.trigger_ticks = m2::sdl::get_ticks() - GAME.pause_ticks;
			}
		} else if (gfx.sprite == &spikes_out && impl.trigger_ticks) {
			// Spikes are out and triggered
			if (1000 < m2::sdl::get_ticks_since(*impl.trigger_ticks, GAME.pause_ticks)) {
				gfx.sprite = &spikes_in;
				impl.trigger_ticks.reset();
			}
		}
	};

	return {};
}
