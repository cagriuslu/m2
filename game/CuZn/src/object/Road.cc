#include <cuzn/object/Road.h>
#include <m2/Game.h>
#include "m2/Log.h"
#include <cuzn/object/Factory.h>
#include <numeric>

m2::Object* find_road_at_location(m2g::pb::SpriteType location) {
	auto roads = M2_LEVEL.characters
				 | std::views::transform(m2::to_character_base)
				 | std::views::filter(is_road_character)
				 | std::views::transform(m2::to_owner_of_component)
				 | std::views::filter(m2::is_object_in_area(std::get<m2::RectF>( M2G_PROXY.connection_positions[location])));
	if (auto road_it = roads.begin(); road_it != roads.end()) {
		return &*road_it;
	}
	return nullptr;
}

void remove_all_roads() {
	std::vector<m2::ObjectId> ids;
	std::ranges::copy(
			M2_LEVEL.characters
				| std::views::transform(m2::to_character_base)
				| std::views::filter(is_road_character)
				| std::views::transform(m2::to_owner_id_of_component),
			std::back_inserter(ids));

	// Delete objects immediately
	std::ranges::for_each(ids, [](m2::ObjectId id) {
		M2_LEVEL.objects.free(id);
	});
}

int link_count_of_road_character(m2::Character& chr) {
	if (not is_road_character(chr)) {
		throw M2_ERROR("Character doesn't belong to canal or railroad");
	}

	auto cities = to_city_cards_of_road_character(chr);
	return std::accumulate(cities.begin(), cities.end(), 0, [](int acc, City city) -> int {
		auto locations = industry_locations_in_city(city);
		return acc + std::accumulate(locations.begin(), locations.end(), 0, [](int acc, IndustryLocation location) -> int {
			if (auto* factory = find_factory_at_location(location)) {
				auto industry_tile = to_industry_tile_of_factory_character(factory->character());
				return acc + m2::iround(M2_GAME.GetNamedItem(industry_tile).get_attribute(m2g::pb::LINK_BONUS));
			}
			return acc;
		});
	});
}

m2::void_expected init_road(m2::Object& obj, Connection connection) {
	DEBUG_FN();

	if (not is_connection(connection)) {
		throw M2_ERROR("Invalid connection");
	}

	// Add the city cards to the character
	auto& chr = obj.add_full_character();
	for (auto city : cities_from_connection(connection)) {
		chr.add_named_item(M2_GAME.GetNamedItem(city));
	}

	auto parent_id = obj.parent_id();
	auto parent_index = M2G_PROXY.player_index(parent_id);
	auto color = M2G_PROXY.player_colors[parent_index];
	auto& _gfx = obj.add_graphic(M2G_PROXY.is_canal_era() ? m2g::pb::SPRITE_CANAL : m2g::pb::SPRITE_RAILROAD);
	_gfx.on_draw = [color](m2::Graphic& gfx) {
		auto connection_position = gfx.owner().position;
		auto cell_rect = m2::RectF{connection_position - 0.75f, 1.5f, 1.5f};

		// Draw background with player's color
		auto background_color = (M2_LEVEL.dimming_exceptions() && not M2_LEVEL.dimming_exceptions()->contains(gfx.owner_id()))
								? color * M2G_PROXY.dimming_factor : color;
		m2::Graphic::color_rect(cell_rect, background_color);

		m2::Graphic::default_draw(gfx); // Draw connection
	};

	return {};
}
