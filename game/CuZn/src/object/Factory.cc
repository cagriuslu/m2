#include <cuzn/object/Factory.h>
#include <m2/Game.h>
#include <cuzn/detail/Graphic.h>
#include <m2/Log.h>

using namespace m2g;

m2::Object* find_factory_at_location(pb::SpriteType location) {
	auto factories = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(is_factory_character)
		| std::views::transform(m2::to_component_parent)
		| std::views::filter(m2::generate_is_object_in_area_filter(M2G_PROXY.industry_positions[location].second));
	if (auto factory_it = factories.begin(); factory_it != factories.end()) {
		return &*factory_it;
	}
	return nullptr;
}

m2::void_expected init_factory(m2::Object& obj, City city, IndustryTile industry_tile) {
	DEBUG_FN();

	if (not is_city(city)) {
		throw M2ERROR("Invalid city");
	}
	if (not is_industry_tile(industry_tile)) {
		throw M2ERROR("Invalid industry tile");
	}

	auto parent_id = obj.parent_id();
	auto parent_index = M2G_PROXY.player_index(parent_id);
	auto industry = industry_of_industry_tile(industry_tile);

	// Add all available information to the factories: industry, city, industry tile
	auto& chr = obj.add_full_character();
	chr.add_named_item(M2_GAME.get_named_item(industry));
	chr.add_named_item(M2_GAME.get_named_item(city));
	const auto& tile_item = M2_GAME.get_named_item(industry_tile);
	chr.add_named_item(tile_item);
	chr.add_resource(pb::COAL_CUBE_COUNT, tile_item.get_attribute(pb::COAL_BONUS));
	chr.add_resource(pb::IRON_CUBE_COUNT, tile_item.get_attribute(pb::IRON_BONUS));
	if (is_canal_era()) {
		chr.add_resource(pb::BEER_BARREL_COUNT, tile_item.get_attribute(pb::BEER_BONUS_FIRST_ERA));
	} else {
		chr.add_resource(pb::BEER_BARREL_COUNT, tile_item.get_attribute(pb::BEER_BONUS_SECOND_ERA));
	}

	auto color = M2G_PROXY.player_colors[parent_index];
	auto& _gfx = obj.add_graphic(industry_sprite_of_industry(industry));
	_gfx.on_draw = [color](m2::Graphic& gfx) {
		auto top_left_cell_pos = gfx.parent().position;
		auto cell_rect = m2::RectF{top_left_cell_pos - 0.5f, 2.0f, 2.0f};
		m2::Graphic::color_rect(cell_rect, color); // Draw background
		m2::Graphic::default_draw(gfx); // Draw industry
		draw_resources(gfx.parent().character()); // Resources
	};

	return {};
}
