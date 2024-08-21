#include <cuzn/object/Factory.h>
#include <m2/Game.h>
#include <cuzn/detail/Graphic.h>
#include <m2/Log.h>

using namespace m2g;
using namespace m2g::pb;

m2::Object* find_factory_at_location(Location location) {
	auto factories = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(is_factory_character)
		| std::views::transform(m2::to_owner_of_component)
		| std::views::filter(m2::is_object_in_area(std::get<m2::RectF>(M2G_PROXY.industry_positions[location])));
	if (auto factory_it = factories.begin(); factory_it != factories.end()) {
		return &*factory_it;
	}
	return nullptr;
}

int required_beer_count_to_sell(IndustryLocation location) {
	if (auto* factory = find_factory_at_location(location); not factory) {
		throw M2_ERROR("Invalid factory location");
	} else {
		auto industry_tile = to_industry_tile_of_factory_character(factory->character());
		return m2::iround(M2_GAME.get_named_item(industry_tile).get_attribute(BEER_COST));
	}
}

void remove_obsolete_factories() {
	std::vector<m2::ObjectId> ids;
	ids.reserve(20); // Reserve an average amount of space
	std::ranges::copy(
		M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(is_factory_character)
		| std::views::filter(is_factory_level_1)
		| std::views::transform(m2::to_owner_id_of_component),
		std::back_inserter(ids));

	// Delete objects immediately
	std::ranges::for_each(ids, [](m2::ObjectId id) {
		M2_LEVEL.objects.free(id);
	});
}

void flip_exhausted_factories() {
	std::ranges::for_each(
		M2_LEVEL.characters
			| std::views::transform(m2::to_character_base)
			| std::views::filter(is_factory_character),
		[](m2::Character& chr) {
			auto is_coal_mine_exhausted = to_industry_of_factory_character(chr) == COAL_MINE_CARD
				&& m2::is_equal(chr.get_resource(COAL_CUBE_COUNT), 0.0f, 0.001f);
			auto is_iron_works_exhausted = to_industry_of_factory_character(chr) == IRON_WORKS_CARD
				&& m2::is_equal(chr.get_resource(IRON_CUBE_COUNT), 0.0f, 0.001f);
			auto is_brewery_exhausted = to_industry_of_factory_character(chr) == BREWERY_CARD
				&& m2::is_equal(chr.get_resource(BEER_BARREL_COUNT), 0.0f, 0.001f);
			if (is_coal_mine_exhausted || is_iron_works_exhausted || is_brewery_exhausted) {
				auto tile_type = to_industry_tile_of_factory_character(chr);
				const auto& tile_item = M2_GAME.get_named_item(tile_type);
				// Earn income points
				auto income_bonus = tile_item.get_attribute(INCOME_POINTS_BONUS);
				auto curr_income_points = chr.owner().character().get_attribute(INCOME_POINTS);
				chr.owner().character().set_attribute(INCOME_POINTS, curr_income_points + income_bonus);
				// Earn victory points
				auto victory_point_bonus = tile_item.get_attribute(VICTORY_POINTS_BONUS);
				auto curr_victory_points = chr.owner().character().get_resource(VICTORY_POINTS);
				chr.owner().character().set_resource(VICTORY_POINTS, curr_victory_points + victory_point_bonus);
				// Flip the tile
				chr.set_resource(IS_SOLD, 1.0f);
			}
		});
}

bool is_factory_sold(m2::Character& chr) {
	if (not is_factory_character(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return m2::is_equal(chr.get_resource(m2g::pb::IS_SOLD), 1.0f, 0.001f);
}

bool is_factory_level_1(m2::Character& chr) {
	static std::set<IndustryTile> level_1_tiles{COTTON_MILL_TILEI, IRON_WORKS_TILE_I, BREWERY_TILE_I, COAL_MINE_TILE_I,
		POTTERY_TILEI, MANUFACTURED_GOODS_TILE_I};
	return level_1_tiles.contains(to_industry_tile_of_factory_character(chr));
}

City to_city_of_factory_character(m2::Character& chr) {
	if (not is_factory_character(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return chr.find_items(m2g::pb::ITEM_CATEGORY_CITY_CARD)->type();
}

Industry to_industry_of_factory_character(m2::Character& chr) {
	if (not is_factory_character(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return chr.find_items(m2g::pb::ITEM_CATEGORY_INDUSTRY_CARD)->type();
}

IndustryTile to_industry_tile_of_factory_character(m2::Character& chr) {
	if (not is_factory_character(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	auto industry = to_industry_of_factory_character(chr);
	auto industry_tile_category = industry_tile_category_of_industry(industry);
	return chr.find_items(industry_tile_category)->type();
}

IndustryLocation to_industry_location_of_factory_character(m2::Character& chr) {
	if (not is_factory_character(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return *industry_location_on_position(chr.owner().position);
}

m2::void_expected init_factory(m2::Object& obj, City city, IndustryTile industry_tile) {
	DEBUG_FN();

	if (not is_city(city)) {
		throw M2_ERROR("Invalid city");
	}
	if (not is_industry_tile(industry_tile)) {
		throw M2_ERROR("Invalid industry tile");
	}

	auto parent_id = obj.parent_id();
	auto parent_index = M2G_PROXY.player_index(parent_id);
	auto industry = industry_of_industry_tile(industry_tile);

	// Add all available information to the factories: industry, city, industry tile
	auto& chr = obj.add_full_character();
	chr.add_named_item(M2_GAME.get_named_item(industry));
	chr.add_named_item(M2_GAME.get_named_item(city));
	chr.add_named_item(M2_GAME.get_named_item(industry_tile));

	auto color = M2G_PROXY.player_colors[parent_index];
	auto& _gfx = obj.add_graphic(industry_sprite_of_industry(industry));
	_gfx.on_draw = [color](m2::Graphic& gfx) {
		auto top_left_cell_pos = gfx.owner().position;
		auto cell_rect = m2::RectF{top_left_cell_pos - 0.5f, 2.0f, 2.0f};

		// Draw background with player's color
		auto background_color = (M2_GAME.dimming_exceptions() && not M2_GAME.dimming_exceptions()->contains(gfx.owner_id()))
			? color * M2G_PROXY.dimming_factor : color;
		m2::Graphic::color_rect(cell_rect, background_color);

		// If sold, draw the black bottom half
		if (m2::is_equal(1.0f, gfx.owner().character().get_resource(m2g::pb::IS_SOLD), 0.005f)) {
			auto bottom_half_cell_rect = m2::RectF{top_left_cell_pos.x - 0.5f, top_left_cell_pos.y + 0.5f, 2.0f, 1.0f};
			m2::Graphic::color_rect(bottom_half_cell_rect, m2::RGB{0, 0, 0});
		}

		// Draw the industry graphic
		m2::Graphic::default_draw(gfx);

		// Draw the resources
		draw_resources(gfx.owner().character());
	};

	return {};
}
