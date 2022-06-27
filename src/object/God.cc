#include <m2/object/God.h>
#include <m2/Game.hh>

std::pair<m2::Object&, m2::ID> m2::obj::create_god() {
	auto obj_pair = create_object(Vec2f{});
	auto& god = obj_pair.first;
	god.impl = std::make_unique<m2::obj::God>();

	auto& monitor = god.add_monitor();
	monitor.pre_phy = [&]([[maybe_unused]] m2::comp::Monitor& mon) {
		m2::Vec2f move_direction;
		if (GAME.events.is_key_down(m2::Key::UP)) {
			move_direction.y += -1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::DOWN)) {
			move_direction.y += 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::LEFT)) {
			move_direction.x += -1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::RIGHT)) {
			move_direction.x += 1.0f;
		}
		god.position += move_direction.normalize() * (GAME.deltaTicks_ms * 1.0f);
	};

	GAME.playerId = obj_pair.second;
	return obj_pair;
}
