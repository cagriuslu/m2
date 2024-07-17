#include <cuzn/object/HumanPlayer.h>
#include <cuzn/journeys/BuildJourney.h>
#include <cuzn/journeys/NetworkJourney.h>
#include <m2/Controls.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/protobuf/Detail.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/Road.h>
#include <cuzn/Detail.h>
#include <ranges>
#include <numeric>

namespace {
	// Filters
	auto by_character_parent_id(m2::Id parent_id) {
		return [parent_id](m2::Character& chr) {
			return chr.parent().parent_id() == parent_id;
		};
	}
}

m2::void_expected init_human_player(m2::Object& obj) {
	DEBUG_FN();

	auto& chr = obj.add_full_character();
	chr.set_resource(m2g::pb::MONEY, 17.0f);
	chr.set_attribute(m2g::pb::INCOME_POINTS, 0.0f);

	// Add industry tiles
	for (auto industry_tile = m2g::pb::COTTON_MILL_TILEI;
	     industry_tile <= m2g::pb::MANUFACTURED_GOODS_TILE_VIII;
	     industry_tile = static_cast<m2g::pb::ItemType>(m2::I(industry_tile) + 1)) {
		// Lookup possession count
		const auto& item = M2_GAME.get_named_item(industry_tile);
		auto possession_limit = m2::I(item.get_attribute(m2g::pb::POSSESSION_LIMIT));
		m2_repeat(possession_limit) { chr.add_named_item(item); }
	}

	// Add connection tiles
	const auto& road_item = M2_GAME.get_named_item(m2g::pb::ROAD_TILE);
	auto road_possession_limit = m2::I(road_item.get_attribute(m2g::pb::POSSESSION_LIMIT));
	m2_repeat(road_possession_limit) { chr.add_named_item(road_item); }

	auto& phy = obj.add_physique();
	phy.pre_step = [&o = obj](MAYBE m2::Physique& _) {
		m2::VecF move_direction;
		if (M2_GAME.events.is_key_down(m2::Key::UP)) {
			move_direction.y -= 1.0f;
		}
		if (M2_GAME.events.is_key_down(m2::Key::DOWN)) {
			move_direction.y += 1.0f;
		}
		if (M2_GAME.events.is_key_down(m2::Key::LEFT)) {
			move_direction.x -= 1.0f;
		}
		if (M2_GAME.events.is_key_down(m2::Key::RIGHT)) {
			move_direction.x += 1.0f;
		}
		o.position += move_direction.normalize() * ((float)M2_GAME.delta_time_s() * M2_GAME.dimensions().height_m);

		if (M2_GAME.events.pop_key_press(m2::Key::MINUS)) {
			M2_GAME.set_zoom(1.1f);  // Increase game height
		} else if (M2_GAME.events.pop_key_press(m2::Key::PLUS)) {
			M2_GAME.set_zoom(0.9f);  // Decrease game height
		}

		// Check if mouse button pressed
		if (M2_GAME.events.pop_mouse_button_press(m2::MouseButton::PRIMARY)) {
			// Check if a user journey is active
			if (auto& user_journey = m2g::Proxy::get_instance().user_journey) {
				// Deliver position signal to current Journey
				std::visit(m2::overloaded{
					[](auto& j) {
						j.signal(PositionOrCancelSignal::create_mouse_click_signal(M2_GAME.mouse_position_world_m()));
					}
				}, *user_journey);
			}
		}
	};

	return {};
}

size_t player_card_count(m2::Character& player) {
	return player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_CITY_CARD)
	+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_WILD_CARD)
	+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_INDUSTRY_CARD);
}

bool player_has_card(m2::Character& player, m2g::pb::ItemType card) {
	return player.find_items(card) != player.end_items();
}

std::list<Card> player_cards(m2::Character& player) {
	std::list<Card> card_list;
	for (auto it = player.begin_items(); it != player.end_items(); ++it) {
		if (it->category() == m2g::pb::ItemCategory::ITEM_CATEGORY_CITY_CARD
			|| it->category() == m2g::pb::ItemCategory::ITEM_CATEGORY_WILD_CARD
			|| it->category() == m2g::pb::ItemCategory::ITEM_CATEGORY_INDUSTRY_CARD) {
			card_list.emplace_back(it->type());
		}
	}
	return card_list;
}

size_t player_road_count(m2::Character& player) {
	return player.count_item(m2g::pb::ROAD_TILE);
}

int player_link_count(m2::Character& player) {
	auto road_characters = M2_LEVEL.characters
					  | std::views::transform(m2::to_character_base)
					  | std::views::filter(by_character_parent_id(player.parent().id()))
					  | std::views::filter(is_road_character);
	return std::accumulate(road_characters.begin(), road_characters.end(), 0, [](int acc, m2::Character& road_char) -> int {
		return acc + link_count_of_road_character(road_char);
	});
}

float player_money(m2::Character& player) {
	return player.get_resource(m2g::pb::MONEY);
}

int player_income_points(m2::Character& player) {
	return m2::iround(player.get_attribute(m2g::pb::INCOME_POINTS));
}

size_t player_tile_count(m2::Character& player) {
	return player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_COAL_MINE_TILE)
		+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_IRON_WORKS_TILE)
		+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_BREWERY_TILE)
		+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_COTTON_MILL_TILE)
		+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_MANUFACTURED_GOODS_TILE)
		+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_POTTERY_TILE);
}

std::optional<m2g::pb::ItemType> get_next_buildable_factory(m2::Character& player, m2g::pb::ItemCategory tile_category) {
	// Find the item with the category with the smallest integer value
	auto tile_item = m2g::pb::ItemType_MAX;
	for (auto item_it = player.find_items(tile_category); item_it != player.end_items(); ++item_it) {
		tile_item = std::min(tile_item, item_it->type());
	}

	if (tile_item == m2g::pb::ItemType_MAX) {
		return std::nullopt;
	} else {
		return tile_item;
	}
}

size_t player_built_factory_count(m2::Character& player) {
	auto factories_view = M2_LEVEL.characters
					   | std::views::transform(m2::to_character_base)
					   | std::views::filter(by_character_parent_id(player.parent().id()))
					   | std::views::filter(is_factory_character);
	return std::distance(factories_view.begin(), factories_view.end());
}

std::set<IndustryLocation> player_built_factory_locations(m2::Character& player) {
	auto factories_view = M2_LEVEL.characters
						  | std::views::transform(m2::to_character_base)
						  | std::views::filter(by_character_parent_id(player.parent().id()))
						  | std::views::filter(is_factory_character)
						  | std::views::transform(to_industry_location_of_factory_character);
	return {factories_view.begin(), factories_view.end()};
}

std::set<m2g::pb::ItemType> get_cities_in_network(m2::Character& player) {
	std::set<m2g::pb::ItemType> cities;

	auto cities_view = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(by_character_parent_id(player.parent().id()))
		| std::views::filter(is_factory_character)
		| std::views::transform(to_city_card_of_factory_character);
	cities.insert(cities_view.begin(), cities_view.end());

	auto roads_view = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(by_character_parent_id(player.parent().id()))
		| std::views::filter(is_road_character)
		| std::views::transform(to_city_cards_of_road_character);
	for (const auto& road_cities : roads_view) {
		std::ranges::copy(road_cities, std::inserter(cities, cities.begin()));
	}

	return cities;
}

std::set<m2g::pb::SpriteType> get_canals_in_network(m2::Character& player) {
	std::set<m2g::pb::SpriteType> canals;

	std::ranges::for_each(get_cities_in_network(player), [&canals](m2g::pb::ItemType city) {
		// Iterate and find all the canals that have the city as one of it's legs
		for (int i = m2g::pb::BELPER_DERBY_CANAL_RAILROAD; i <= m2g::pb::REDDITCH_OXFORD_CANAL_RAILROAD; ++i) {
			auto road_location_type = static_cast<m2g::pb::SpriteType>(i);
			const auto& road_location = M2_GAME.get_sprite(road_location_type);
			if (std::ranges::any_of(road_location.named_items(), is_canal_license) &&
				std::ranges::count(road_location.named_items(), city)) {
				canals.insert(road_location_type);
			}
		}
	});

	return canals;
}

std::set<m2g::pb::SpriteType> get_railroads_in_network(m2::Character& player) {
	std::set<m2g::pb::SpriteType> railroads;

	std::ranges::for_each(get_cities_in_network(player), [&railroads](m2g::pb::ItemType city) {
		// Iterate and find all the railroads that have the city as one of it's legs
		for (int i = m2g::pb::BELPER_DERBY_CANAL_RAILROAD; i <= m2g::pb::REDDITCH_OXFORD_CANAL_RAILROAD; ++i) {
			auto road_location_type = static_cast<m2g::pb::SpriteType>(i);
			const auto& road_location = M2_GAME.get_sprite(road_location_type);
			if (std::ranges::any_of(road_location.named_items(), is_railroad_license) &&
				std::ranges::count(road_location.named_items(), city)) {
				railroads.insert(road_location_type);
			}
		}
	});

	return railroads;
}
