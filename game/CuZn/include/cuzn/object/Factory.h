#pragma once

#include <m2/Object.h>
#include <cuzn/Detail.h>

// Accessors
m2::Object* find_factory_at_location(Location location);

// Global Modifiers
void remove_obsolete_factories();
void flip_exhausted_factories();

// Filters
bool is_factory_character(m2::Character& chr);
bool is_factory_sold(m2::Character& chr);
bool is_factory_level_1(m2::Character& chr);

// Transformers
City to_city_of_factory_character(m2::Character& chr);
Industry to_industry_of_factory_character(m2::Character& chr);
IndustryTile to_industry_tile_of_factory_character(m2::Character& chr);
IndustryLocation to_industry_location_of_factory_character(m2::Character& chr);

m2::void_expected init_factory(m2::Object&, City, IndustryTile);
