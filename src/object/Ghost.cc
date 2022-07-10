#include <m2/object/Ghost.h>
#include <m2/Game.hh>
#include <m2g/SpriteBlueprint.h>

namespace {
	m2::Vec2f calculate_ghost_position() {
		auto mouse_pos_m = GAME.mousePositionWRTGameWorld_m;
		auto floored_mouse_pos_m = m2::Vec2f{floor(mouse_pos_m.x), floor(mouse_pos_m.y)};
		return floored_mouse_pos_m + 0.5f;
	}

	std::pair<m2::Object&, m2::ID> create_ghost(m2::SpriteIndex sprite_index) {
		auto obj_pair = m2::create_object(calculate_ghost_position());
		auto& ghost = obj_pair.first;

		auto& gfx = ghost.add_graphic();
		gfx.textureRect = m2g::sprites[sprite_index].texture_rect;
		gfx.center_px = m2g::sprites[sprite_index].obj_center_px;

		auto& monitor = ghost.add_monitor();
		monitor.pre_phy = [&ghost](m2::comp::Monitor& mon) {
			auto pos = calculate_ghost_position();
			ghost.position = pos;
		};

		return obj_pair;
	}
}

void m2::obj::set_editor_ghost(SpriteIndex sprite_index) {
	if (GAME.level->editor_draw_ghost_id) {
		GAME.add_deferred_action(m2::create_object_deleter(GAME.level->editor_draw_ghost_id));
	}
	if (sprite_index) {
		auto ghost_pair = create_ghost(sprite_index);
		GAME.level->editor_draw_ghost_id = ghost_pair.second;
	}
}
