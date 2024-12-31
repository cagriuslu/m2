#include <cuzn/object/HumanPlayer.h>
#include <cuzn/journeys/BuildJourney.h>
#include <cuzn/journeys/NetworkJourney.h>
#include <cuzn/ui/MouseHover.h>
#include <m2/Controls.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/protobuf/Detail.h>
#include <cuzn/object/Factory.h>
#include <cuzn/object/Road.h>
#include <cuzn/Detail.h>
#include <ranges>
#include <numeric>
#include <cuzn/ConsumingCoal.h>
#include <cuzn/ConsumingIron.h>

struct HumanPlayer : public m2::ObjectImpl {
	std::optional<std::pair<m2::VecI, m2::VecF>> mouse_click_prev_position;
	std::optional<Location> currentMouseHoverLocation;
};

m2::void_expected InitThisHumanPlayerInstance(m2::Object& obj) {
	DEBUG_FN();

	obj.impl = std::make_unique<HumanPlayer>();

	auto& chr = obj.add_full_character();
	chr.set_resource(m2g::pb::MONEY, 17.0f);
	chr.set_attribute(m2g::pb::INCOME_POINTS, 0.0f);

	// Add industry tiles
	for (auto industry_tile = m2g::pb::COTTON_MILL_TILE_I;
	     industry_tile <= m2g::pb::MANUFACTURED_GOODS_TILE_VIII;
	     industry_tile = static_cast<m2g::pb::ItemType>(m2::I(industry_tile) + 1)) {
		// Lookup possession count
		const auto& item = M2_GAME.GetNamedItem(industry_tile);
		auto possession_limit = m2::I(item.get_attribute(m2g::pb::POSSESSION_LIMIT));
		m2_repeat(possession_limit) { chr.add_named_item(item); }
	}

	// Add connection tiles
	const auto& road_item = M2_GAME.GetNamedItem(m2g::pb::ROAD_TILE);
	auto road_possession_limit = m2::I(road_item.get_attribute(m2g::pb::POSSESSION_LIMIT));
	m2_repeat(road_possession_limit) { chr.add_named_item(road_item); }

	auto& phy = obj.add_physique();
	phy.pre_step = [&o = obj](MAYBE m2::Physique& _) {
		auto& impl = dynamic_cast<HumanPlayer&>(*o.impl);
		// Start map movement with mouse
		if (M2_GAME.events.pop_mouse_button_press(m2::MouseButton::PRIMARY, M2_GAME.Dimensions().Game())) {
			LOG_DEBUG("Begin panning");
			impl.mouse_click_prev_position = std::make_pair(M2_GAME.events.mouse_position(), M2_GAME.MousePositionWorldM());
			M2_LEVEL.enable_panning();
		} else if (impl.mouse_click_prev_position && not M2_GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY)) {
			LOG_DEBUG("End panning");
			impl.mouse_click_prev_position.reset();
			M2_LEVEL.disable_panning();
		}
		// Map movement is enabled
		if (impl.mouse_click_prev_position && impl.mouse_click_prev_position->first != M2_GAME.events.mouse_position()) {
			auto diff = impl.mouse_click_prev_position->first - M2_GAME.events.mouse_position();
			auto diff_m = m2::VecF{diff} / M2_GAME.Dimensions().RealOutputPixelsPerMeter();
			o.position += diff_m;
			impl.mouse_click_prev_position = std::make_pair(M2_GAME.events.mouse_position(), M2_GAME.MousePositionWorldM());
		}

		constexpr float zoom_step = 1.1f;
		if (auto scroll = M2_GAME.events.pop_mouse_wheel_vertical_scroll(M2_GAME.Dimensions().Game()); 0 < scroll) {
			M2_GAME.SetScale(M2_GAME.Dimensions().Scale() * zoom_step);
		} else if (scroll < 0) {
			M2_GAME.SetScale(M2_GAME.Dimensions().Scale() / zoom_step);
		}

		// Limit the player inside the level
		const auto& dims = M2_GAME.Dimensions();
		// If the map is zoomed out so much that the black space is showing on the left and the right
		if (M2_LEVEL.background_boundary().w < dims.GameM().x) {
			o.position.x = M2_LEVEL.background_boundary().x_center();
		} else {
			if (o.position.x < dims.GameM().x / 2.0f) {
				o.position.x = dims.GameM().x / 2.0f; // Left
			}
			if (M2_LEVEL.background_boundary().x2() < o.position.x + dims.GameM().x / 2.0f) {
				o.position.x = M2_LEVEL.background_boundary().x2() - dims.GameM().x / 2.0f; // Right
			}
		}
		// If the map is zoomed out so much that the black space is showing on the top and the bottom
		if (M2_LEVEL.background_boundary().h < dims.GameM().y) {
			o.position.y = M2_LEVEL.background_boundary().y_center();
		} else {
			if (o.position.y < dims.GameM().y / 2.0f) {
				o.position.y = dims.GameM().y / 2.0f; // Top
			}
			if (M2_LEVEL.background_boundary().y2() < o.position.y + dims.GameM().y / 2.0f) {
				o.position.y = M2_LEVEL.background_boundary().y2() - dims.GameM().y / 2.0f; // Bottom
			}
		}

		// Check if mouse button pressed
		if (M2_GAME.events.pop_mouse_button_press(m2::MouseButton::SECONDARY)) {
			if (M2G_PROXY.main_journeys) {
				std::visit(m2::overloaded{
					[](auto& journey) { journey.sub_journey->signal(PositionOrCancelSignal::create_mouse_click_signal(M2_GAME.MousePositionWorldM())); }
				}, *M2G_PROXY.main_journeys);
			}
		}

		// Mouse hover UI panel
		const auto mousePositionInWorld = M2_GAME.MousePositionWorldM();
		if (const auto industry_location = industry_location_on_position(mousePositionInWorld)) {
			if (not impl.currentMouseHoverLocation || *impl.currentMouseHoverLocation != industry_location) {
				if (impl.currentMouseHoverLocation) {
					M2_LEVEL.RemoveMouseHoverUiPanel();
					impl.currentMouseHoverLocation.reset();
				}
				// Look up factory if exists, otherwise the background sprite
				if (FindFactoryAtLocation(*industry_location)) {
					auto [bp, rectf] = GenerateBuiltIndustryLocationMouseHoverUiBlueprint(*industry_location);
					M2_LEVEL.AddMouseHoverUiPanel(std::make_unique<m2::ui::PanelBlueprint>(bp), rectf);
					impl.currentMouseHoverLocation = *industry_location;
				} else {
					auto [bp, rectf] = GenerateEmptyIndustryLocationMouseHoverUiBlueprint(*industry_location);
					M2_LEVEL.AddMouseHoverUiPanel(std::make_unique<m2::ui::PanelBlueprint>(bp), rectf);
					impl.currentMouseHoverLocation = *industry_location;
				}
			}
		} else if (const auto merchant_location = merchant_location_on_position(mousePositionInWorld)) {
			if (not impl.currentMouseHoverLocation || *impl.currentMouseHoverLocation != merchant_location) {
				if (impl.currentMouseHoverLocation) {
					M2_LEVEL.RemoveMouseHoverUiPanel();
					impl.currentMouseHoverLocation.reset();
				}
				auto [bp, rectf] = GenerateMerchantLocationMouseHoverUiBlueprint(*merchant_location);
				M2_LEVEL.AddMouseHoverUiPanel(std::make_unique<m2::ui::PanelBlueprint>(bp), rectf);
				impl.currentMouseHoverLocation = *merchant_location;
			}
		} else if (const auto connection = connection_on_position(mousePositionInWorld)) {
			if (not impl.currentMouseHoverLocation || *impl.currentMouseHoverLocation != connection) {
				if (impl.currentMouseHoverLocation) {
					M2_LEVEL.RemoveMouseHoverUiPanel();
					impl.currentMouseHoverLocation.reset();
				}
				auto [bp, rectf] = GenerateConnectionMouseHoverUiBlueprint(*connection);
				M2_LEVEL.AddMouseHoverUiPanel(std::make_unique<m2::ui::PanelBlueprint>(bp), rectf);
				impl.currentMouseHoverLocation = *connection;
			}
		} else {
			// Remove mouse hover UI panel if activated
			if (impl.currentMouseHoverLocation) {
				M2_LEVEL.RemoveMouseHoverUiPanel();
				impl.currentMouseHoverLocation.reset();
			}
		}
	};

	// Set the player ID so that the camera can find this
	M2_LEVEL.player_id = obj.id();

	return {};
}
m2::void_expected InitOtherHumanPlayerInstance(m2::Object& obj) {
	DEBUG_FN();

	auto& chr = obj.add_full_character();

	// TODO check if the following is really necessary. If the ServerUpdate is verified, it's necessary.
	// TODO Otherwise, we don't need to fill the character with items and resources.

	chr.set_resource(m2g::pb::MONEY, 17.0f);
	chr.set_attribute(m2g::pb::INCOME_POINTS, 0.0f);

	// Add industry tiles
	for (auto industry_tile = m2g::pb::COTTON_MILL_TILE_I;
		 industry_tile <= m2g::pb::MANUFACTURED_GOODS_TILE_VIII;
		 industry_tile = static_cast<m2g::pb::ItemType>(m2::I(industry_tile) + 1)) {
		// Lookup possession count
		const auto& item = M2_GAME.GetNamedItem(industry_tile);
		auto possession_limit = m2::I(item.get_attribute(m2g::pb::POSSESSION_LIMIT));
		m2_repeat(possession_limit) { chr.add_named_item(item); }
		 }

	// Add connection tiles
	const auto& road_item = M2_GAME.GetNamedItem(m2g::pb::ROAD_TILE);
	auto road_possession_limit = m2::I(road_item.get_attribute(m2g::pb::POSSESSION_LIMIT));
	m2_repeat(road_possession_limit) { chr.add_named_item(road_item); }

	return {};
}

size_t PlayerCardCount(m2::Character& player) {
	return player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_CITY_CARD)
	+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_WILD_CARD)
	+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_INDUSTRY_CARD);
}

std::list<Card> PlayerCards(m2::Character& player) {
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

int PlayerLinkCount(m2::Character& player) {
	auto road_characters = M2_LEVEL.characters
					  | std::views::transform(m2::to_character_base)
					  | std::views::filter(m2::is_component_of_parent_object(player.owner_id()))
					  | std::views::filter(is_road_character);
	return std::accumulate(road_characters.begin(), road_characters.end(), 0, [](int acc, m2::Character& road_char) -> int {
		return acc + link_count_of_road_character(road_char);
	});
}

int PlayerVictoryPoints(m2::Character& player) {
	return m2::iround(player.get_resource(m2g::pb::VICTORY_POINTS));
}

int PlayerIncomePoints(m2::Character& player) {
	return m2::iround(player.get_attribute(m2g::pb::INCOME_POINTS));
}

int PlayerMoney(m2::Character& player) {
	return m2::iround(player.get_resource(m2g::pb::MONEY));
}

size_t player_tile_count(m2::Character& player) {
	return player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_COAL_MINE_TILE)
		+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_IRON_WORKS_TILE)
		+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_BREWERY_TILE)
		+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_COTTON_MILL_TILE)
		+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_MANUFACTURED_GOODS_TILE)
		+ player.count_item(m2g::pb::ItemCategory::ITEM_CATEGORY_POTTERY_TILE);
}

std::optional<m2g::pb::ItemType> get_next_industry_tile_of_category(m2::Character& player, m2g::pb::ItemCategory tile_category) {
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
std::optional<m2g::pb::ItemType> get_next_industry_tile_of_industry(m2::Character& player, const Industry industry) {
	return get_next_industry_tile_of_category(player, industry_tile_category_of_industry(industry));
}

size_t player_built_factory_count(m2::Character& player) {
	auto factories_view = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(m2::is_component_of_parent_object(player.owner_id()))
		| std::views::filter(IsFactoryCharacter);
	return std::distance(factories_view.begin(), factories_view.end());
}

std::set<IndustryLocation> player_built_factory_locations(m2::Character& player) {
	auto factories_view = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(m2::is_component_of_parent_object(player.owner_id()))
		| std::views::filter(IsFactoryCharacter)
		| std::views::transform(ToIndustryLocationOfFactoryCharacter);
	return {factories_view.begin(), factories_view.end()};
}

std::set<IndustryLocation> player_sellable_factory_locations(m2::Character& player) {
	auto factories_view = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(m2::is_component_of_parent_object(player.owner_id()))
		| std::views::filter(IsFactoryCharacter)
		| std::views::filter(IsFactoryNotSold)
		| std::views::filter([](const m2::Character& c) {
			return is_sellable_industry(ToIndustryOfFactoryCharacter(c));
		})
		| std::views::transform(ToIndustryLocationOfFactoryCharacter);
	return {factories_view.begin(), factories_view.end()};
}

m2::void_expected can_player_overbuild_on_location_with_card(m2::Character& player, IndustryLocation location, Card card) {
	// Check the industry type of the already built factory
	auto* factory = FindFactoryAtLocation(location);
	auto industry_of_factory = ToIndustryOfFactoryCharacter(factory->character());

	// Check if the selected card can build the same industry
	if (card != m2g::pb::WILD_LOCATION_CARD && card != m2g::pb::WILD_INDUSTRY_CARD & card != industry_of_factory) {
		return m2::make_unexpected("Selected card cannot overbuild the same industry type");
	}

	// Check if the factory belongs to the player
	if (player.owner_id() == factory->parent_id()) {
		// Any industry can be overbuilt
		return {};
	}

	// Only coal mine or iron works can be overbuilt
	if (industry_of_factory != m2g::pb::COAL_MINE_CARD && industry_of_factory != m2g::pb::IRON_WORKS_CARD) {
		return m2::make_unexpected("Selected industry type cannot be overbuilt when it belongs to another player");
	}
	// There must be no resources left on the whole board of the type of the resource of the factory
	if (industry_of_factory == m2g::pb::COAL_MINE_CARD) {
		if (is_there_coal_on_the_board()) {
			return m2::make_unexpected("Cannot overbuild coal mine while there are still coal on the board");
		}
	} else { // industry_of_factory == m2g::pb::IRON_WORKS_CARD
		if (is_there_iron_on_the_board()) {
			return m2::make_unexpected("Cannot overbuild iron works while there are still iron on the board");
		}
	}
	return {};
}

std::set<m2g::pb::ItemType> get_cities_in_network(m2::Character& player) {
	std::set<m2g::pb::ItemType> cities;

	auto cities_view = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(m2::is_component_of_parent_object(player.owner_id()))
		| std::views::filter(IsFactoryCharacter)
		| std::views::transform(ToCityOfFactoryCharacter);
	cities.insert(cities_view.begin(), cities_view.end());

	auto roads_view = M2_LEVEL.characters
		| std::views::transform(m2::to_character_base)
		| std::views::filter(m2::is_component_of_parent_object(player.owner_id()))
		| std::views::filter(is_road_character)
		| std::views::transform(to_city_cards_of_road_character);
	for (const auto& road_cities : roads_view) {
		std::ranges::copy(road_cities, std::inserter(cities, cities.begin()));
	}

	return cities;
}

std::set<m2g::pb::SpriteType> get_canals_in_network(m2::Character& player, Connection provisional_extra_connection) {
	std::set<m2g::pb::SpriteType> canals;

	auto cities_in_network = get_cities_in_network(player);
	if (provisional_extra_connection) {
		auto extra_cities = cities_from_connection(provisional_extra_connection);
		cities_in_network.insert(extra_cities.begin(), extra_cities.end());
	}

	std::ranges::for_each(cities_in_network, [&canals](m2g::pb::ItemType city) {
		// Iterate and find all the canals that have the city as one of it's legs
		for (int i = m2g::pb::BELPER_DERBY_CANAL_RAILROAD; i <= m2g::pb::REDDITCH_OXFORD_CANAL_RAILROAD; ++i) {
			auto road_location_type = static_cast<m2g::pb::SpriteType>(i);
			const auto& road_location = M2_GAME.GetSprite(road_location_type);
			if (std::ranges::any_of(road_location.NamedItems(), is_canal_license) &&
				std::ranges::count(road_location.NamedItems(), city)) {
				canals.insert(road_location_type);
			}
		}
	});

	return canals;
}

std::set<m2g::pb::SpriteType> get_railroads_in_network(m2::Character& player, Connection provisional_extra_connection) {
	std::set<m2g::pb::SpriteType> railroads;

	auto cities_in_network = get_cities_in_network(player);
	if (provisional_extra_connection) {
		auto extra_cities = cities_from_connection(provisional_extra_connection);
		cities_in_network.insert(extra_cities.begin(), extra_cities.end());
	}

	std::ranges::for_each(cities_in_network, [&railroads](m2g::pb::ItemType city) {
		// Iterate and find all the railroads that have the city as one of it's legs
		for (int i = m2g::pb::BELPER_DERBY_CANAL_RAILROAD; i <= m2g::pb::REDDITCH_OXFORD_CANAL_RAILROAD; ++i) {
			auto road_location_type = static_cast<m2g::pb::SpriteType>(i);
			const auto& road_location = M2_GAME.GetSprite(road_location_type);
			if (std::ranges::any_of(road_location.NamedItems(), is_railroad_license) &&
				std::ranges::count(road_location.NamedItems(), city)) {
				railroads.insert(road_location_type);
			}
		}
	});

	return railroads;
}

std::set<m2g::pb::SpriteType> get_connections_in_network(m2::Character& player, Connection provisional_extra_connection) {
	if (M2G_PROXY.is_canal_era()) {
		return get_canals_in_network(player, provisional_extra_connection);
	} else {
		return get_railroads_in_network(player, provisional_extra_connection);
	}
}
