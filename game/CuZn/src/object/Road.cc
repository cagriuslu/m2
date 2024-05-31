#include <cuzn/object/Road.h>
#include <m2/Game.h>
#include "m2/Log.h"

m2::Object* find_road_at_location(m2g::pb::SpriteType location) {
	auto roads = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(is_road_character)
		| std::views::transform(m2::to_character_parent)
		| std::views::filter(m2::is_object_in_area(M2G_PROXY.connection_positions[location].second));
	if (auto road_it = roads.begin(); road_it != roads.end()) {
		return &*road_it;
	}
	return nullptr;
}

m2::void_expected init_road(m2::Object& obj, Connection connection) {
	DEBUG_FN();

	if (not is_connection(connection)) {
		throw M2ERROR("Invalid connection");
	}

	// Add the city cards to the character
	auto& chr = obj.add_full_character();
	for (auto city : cities_from_connection(connection)) {
		chr.add_named_item(M2_GAME.get_named_item(city));
	}

	auto parent_id = obj.parent_id();
	auto parent_index = M2G_PROXY.player_index(parent_id);
	auto color = M2G_PROXY.player_colors[parent_index];
	auto& _gfx = obj.add_graphic(is_canal_era() ? m2g::pb::SPRITE_CANAL : m2g::pb::SPRITE_RAILROAD);
	_gfx.on_draw = [color](m2::Graphic& gfx) {
		auto connection_position = gfx.parent().position;
		auto cell_rect = m2::RectF{connection_position - 0.75f, 1.5f, 1.5f};
		m2::Graphic::color_rect(cell_rect, color); // Draw background
		m2::Graphic::default_draw(gfx); // Draw connection
	};

	return {};
}
