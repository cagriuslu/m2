#include <cuzn/object/HumanPlayer.h>
#include <m2/Controls.h>
#include <m2/Game.h>
#include <m2/protobuf/Detail.h>
#include <cuzn/object/Tile.h>
#include <cuzn/object/CanalOrRailroad.h>
#include <ranges>

namespace {
	// Filters
	auto by_character_parent_id(m2::Id parent_id) {
		return [parent_id](m2::Character* chr) {
			return chr->parent().parent_id() == parent_id;
		};
	}
}

m2::void_expected cuzn::init_human_player(m2::Object& obj) {
	DEBUG_FN();

	auto& chr = obj.add_full_character();
	chr.add_resource(m2g::pb::MONEY, 17.0f);
	chr.set_resource(m2g::pb::ERA, 1.0f); // Set Era as 1 during creation
	chr.set_attribute(m2g::pb::VICTORY_POINTS, 0.0f);
	chr.set_attribute(m2g::pb::INCOME_POINTS, 0.0f);

	// Add industry tiles
	for (auto industry_tile = m2g::pb::COTTON_MILL_TILEI; industry_tile <= m2g::pb::MANUFACTURED_GOODS_TILE_VIII;
		industry_tile = static_cast<m2g::pb::ItemType>(m2::I(industry_tile) + 1)) {
		// Lookup possession count
		auto& item = GAME.get_named_item(industry_tile);
		auto possession_limit = m2::I(item.get_attribute(m2g::pb::POSSESSION_LIMIT));
		m2_repeat(possession_limit) { chr.add_named_item(item); }
	}

	auto& phy = obj.add_physique();
	phy.pre_step = [&o = obj](MAYBE m2::Physique& _) {
		m2::VecF move_direction;
		if (GAME.events.is_key_down(m2::Key::UP)) {
			move_direction.y -= 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::DOWN)) {
			move_direction.y += 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::LEFT)) {
			move_direction.x -= 1.0f;
		}
		if (GAME.events.is_key_down(m2::Key::RIGHT)) {
			move_direction.x += 1.0f;
		}
		o.position += move_direction.normalize() * ((float)GAME.delta_time_s() * GAME.dimensions().height_m);

		// Check if mouse button pressed
		if (GAME.events.pop_mouse_button_press(m2::MouseButton::PRIMARY)) {
			// Check if a user journey is active
			if (auto& user_journey = m2g::Proxy::get_instance().user_journey; user_journey) {
				// Check if BuildJourney is active
				if (std::holds_alternative<BuildJourney>(*user_journey)) {
					// Deliver mouse click to BuildJourney
					std::get<BuildJourney>(*user_journey).signal(PositionOrCancelSignal::create_mouse_click_signal(GAME.mouse_position_world_m()));
				}
			}
		}
	};

	return {};
}

std::optional<m2g::pb::ItemType> cuzn::get_next_buildable_tile(m2::Character& player, m2g::pb::ItemCategory tile_category) {
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

std::set<m2g::pb::ItemType> cuzn::get_cities_in_network(m2::Character& player) {
	std::set<m2g::pb::ItemType> cities;

	auto cities_range = LEVEL.characters
		| std::views::transform(m2::to_only_data)
		| std::views::transform(m2::to_character_base)
		| std::views::filter(by_character_parent_id(player.parent().id()))
		| std::views::filter(is_tile_character)
		| std::views::transform(to_first_city_card_of_tile_character);
	cities.insert(cities_range.begin(), cities_range.end());

	auto canals_or_railroads_range = LEVEL.characters
		| std::views::transform(m2::to_only_data)
		| std::views::transform(m2::to_character_base)
		| std::views::filter(by_character_parent_id(player.parent().id()))
		| std::views::filter(is_canal_or_railroad_character)
		| std::views::transform(to_first_two_city_cards_of_canal_or_railroad_character);
	for (const auto& cards : canals_or_railroads_range) {
		cities.emplace(cards.first);
		cities.emplace(cards.second);
	}

	return cities;
}

std::set<m2g::pb::ItemType> cuzn::get_canals_in_network(m2::Character& player) {
	// TODO
}

std::set<m2g::pb::ItemType> cuzn::get_railroads_in_network(m2::Character& player) {
	// TODO
}
