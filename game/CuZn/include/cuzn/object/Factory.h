#pragma once

#include <m2/Object.h>
#include <cuzn/Detail.h>

// Accessors
m2::Object* find_factory_at_location(m2g::pb::SpriteType location);

// Modifiers
void remove_obsolete_factories();

// Filters
constexpr auto is_factory_character = [](m2::Character& chr) { return chr.parent().object_type() == m2g::pb::FACTORY; };
constexpr auto is_factory_sold = [](m2::Character& chr) {
	if (not is_factory_character(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return m2::is_equal(chr.get_resource(m2g::pb::IS_SOLD), 1.0f, 0.001f);
};

// Transformers
constexpr auto to_city_card_of_factory_character = [](m2::Character& chr) {
	if (not is_factory_character(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return chr.find_items(m2g::pb::ITEM_CATEGORY_CITY_CARD)->type();
};
constexpr auto to_industry_card_of_factory_character = [](m2::Character& chr) {
	if (not is_factory_character(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return chr.find_items(m2g::pb::ITEM_CATEGORY_INDUSTRY_CARD)->type();
};
constexpr auto to_industry_tile_of_factory_character = [](m2::Character& chr) {
	if (not is_factory_character(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	auto industry = to_industry_card_of_factory_character(chr);
	auto industry_tile_category = industry_tile_category_of_industry(industry);
	return chr.find_items(industry_tile_category)->type();
};

// Filter
constexpr auto is_factory_level_1 = [](m2::Character& chr) {
	return is_industry_tile_level_1(to_industry_tile_of_factory_character(chr));
};

m2::void_expected init_factory(m2::Object&, City, IndustryTile);
