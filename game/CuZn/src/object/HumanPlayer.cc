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
#include <m2/ObjectEx.h>
#include <cuzn/ConsumingCoal.h>
#include <cuzn/ConsumingIron.h>

struct HumanPlayer : public m2::HeapObjectImpl {
	std::optional<Location> currentMouseHoverLocation;

	explicit HumanPlayer(m2::Object&) : HeapObjectImpl() {}
};

m2::void_expected PlayerInitThisInstance(m2::Object& obj, const m2::VecF& position) {
	DEBUG_FN();

	obj.impl = std::make_unique<HumanPlayer>(obj);

	auto& chr = m2::AddCharacterToObject<m2g::ProxyEx::FastCharacterStorageIndex>(obj);
	chr.UnsafeSetVariable(m2g::pb::MONEY, m2::VariableValue{17});
	chr.UnsafeSetVariable(m2g::pb::INCOME_POINTS, m2::VariableValue{0});

	// Add industry tiles
	for (auto industry_tile = m2g::pb::COTTON_MILL_TILE_I;
	     industry_tile <= m2g::pb::MANUFACTURED_GOODS_TILE_VIII;
	     industry_tile = static_cast<m2g::pb::CardType>(m2::I(industry_tile) + 1)) {
		// Lookup possession count
		const auto& card = M2_GAME.GetCard(industry_tile);
		auto possession_limit = card.GetConstant(m2g::pb::POSSESSION_LIMIT).GetIntOrZero();
		m2Repeat(possession_limit) { chr.UnsafeAddCard(industry_tile); }
	}

	// Add connection tiles
	const auto& road_card = M2_GAME.GetCard(m2g::pb::ROAD_TILE);
	auto road_possession_limit = road_card.GetConstant(m2g::pb::POSSESSION_LIMIT).GetIntOrZero();
	m2Repeat(road_possession_limit) { chr.UnsafeAddCard(m2g::pb::ROAD_TILE); }

	auto& phy = obj.AddPhysique();
	phy.position = position;
	phy.preStep = [&o = obj](m2::Physique& phy_, const m2::Stopwatch::Duration&) {
		auto& impl = dynamic_cast<HumanPlayer&>(*std::get<std::unique_ptr<m2::HeapObjectImpl>>(o.impl));
		// Start map movement with mouse
		if (M2_GAME.events.PopMouseButtonPress(m2::MouseButton::PRIMARY, M2_GAME.Dimensions().Game())) {
			LOG_TRACE("Begin panning");
			M2_LEVEL.BeginPanning();
		} else if (M2_LEVEL.IsPanning() && not M2_GAME.events.IsMouseButtonDown(m2::MouseButton::PRIMARY)) {
			LOG_TRACE("End panning");
			M2_LEVEL.EndPanning();
		}
		// Map movement is enabled
		if (const auto panBeginPosition = M2_LEVEL.GetPanBeginPosition(); panBeginPosition && panBeginPosition->first != M2_GAME.events.MousePosition()) {
			auto diff = panBeginPosition->first - M2_GAME.events.MousePosition();
			auto diff_m = m2::VecF{diff} / M2_GAME.Dimensions().OutputPixelsPerMeter();
			phy_.position += diff_m;
			M2_LEVEL.BeginPanning();
		}

		constexpr float zoom_step = 1.2f;
		if (auto scroll = M2_GAME.events.PopMouseWheelVerticalScroll(M2_GAME.Dimensions().Game()); 0 < scroll) {
			if (20.0f < M2_GAME.Dimensions().GameM().GetY()) {
				M2_GAME.SetScale(M2_GAME.Dimensions().Scale() * zoom_step);
			}
		} else if (scroll < 0) {
			if (M2_GAME.Dimensions().GameM().GetY() < 55.0f) {
				M2_GAME.SetScale(M2_GAME.Dimensions().Scale() / zoom_step);
			}
		}

		// Limit the player inside the level
		const auto& dims = M2_GAME.Dimensions();
		// If the map is zoomed out so much that the black space is showing on the left and the right
		if (M2_LEVEL.GetBackgroundBoundary().w < dims.GameM().GetX()) {
			phy_.position = phy_.position.WithX(m2::ToFloat(M2_LEVEL.GetBackgroundBoundary().GetXCenter()));
		} else {
			if (phy_.position.GetX() < dims.GameM().GetX() / 2.0f) {
				phy_.position = phy_.position.WithX(dims.GameM().GetX() / 2.0f); // Left
			}
			if (M2_LEVEL.GetBackgroundBoundary().GetX2() < phy_.position.GetX() + dims.GameM().GetX() / 2.0f) {
				phy_.position = phy_.position.WithX(M2_LEVEL.GetBackgroundBoundary().GetX2() - dims.GameM().GetX() / 2.0f); // Right
			}
		}
		// If the map is zoomed out so much that the black space is showing on the top and the bottom
		if (M2_LEVEL.GetBackgroundBoundary().h < dims.GameM().GetY()) {
			phy_.position = phy_.position.WithY(m2::ToFloat(M2_LEVEL.GetBackgroundBoundary().GetYCenter()));
		} else {
			if (phy_.position.GetY() < dims.GameM().GetY() / 2.0f) {
				phy_.position = phy_.position.WithY(dims.GameM().GetY() / 2.0f); // Top
			}
			if (M2_LEVEL.GetBackgroundBoundary().GetY2() < phy_.position.GetY() + dims.GameM().GetY() / 2.0f) {
				phy_.position = phy_.position.WithY(M2_LEVEL.GetBackgroundBoundary().GetY2() - dims.GameM().GetY() / 2.0f); // Bottom
			}
		}

		// Check if mouse button pressed
		if (M2_GAME.events.PopMouseButtonPress(m2::MouseButton::SECONDARY)) {
			if (M2G_PROXY.main_journeys) {
				std::visit(m2::overloaded{
					[](auto& journey) { journey.sub_journey->signal(PositionOrCancelSignal::create_mouse_click_signal(M2_GAME.MousePositionWorldM())); }
				}, *M2G_PROXY.main_journeys);
			}
		}

		// Mouse hover UI panel
		const auto mousePositionInWorld = M2_GAME.MousePositionWorldM();
		if (not M2_GAME.IsMouseOnAnyUiPanel()) {
			if (const auto industry_location = industry_location_on_position(mousePositionInWorld)) {
				if (not impl.currentMouseHoverLocation || *impl.currentMouseHoverLocation != industry_location) {
					if (impl.currentMouseHoverLocation) {
						M2_LEVEL.RemoveMouseHoverUiPanel();
						impl.currentMouseHoverLocation.reset();
					}
					// Look up factory if exists, otherwise the background sprite
					if (FindFactoryAtLocation(*industry_location)) {
						auto [bp, rectf] = GenerateBuiltIndustryLocationMouseHoverUiBlueprint(*industry_location);
						M2_LEVEL.SetMouseHoverUiPanel(std::make_unique<m2::UiPanelBlueprint>(bp), rectf);
						impl.currentMouseHoverLocation = *industry_location;
					} else {
						auto [bp, rectf] = GenerateEmptyIndustryLocationMouseHoverUiBlueprint(*industry_location);
						M2_LEVEL.SetMouseHoverUiPanel(std::make_unique<m2::UiPanelBlueprint>(bp), rectf);
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
					M2_LEVEL.SetMouseHoverUiPanel(std::make_unique<m2::UiPanelBlueprint>(bp), rectf);
					impl.currentMouseHoverLocation = *merchant_location;
				}
			} else if (const auto connection = connection_on_position(mousePositionInWorld)) {
				if (not impl.currentMouseHoverLocation || *impl.currentMouseHoverLocation != connection) {
					if (impl.currentMouseHoverLocation) {
						M2_LEVEL.RemoveMouseHoverUiPanel();
						impl.currentMouseHoverLocation.reset();
					}
					auto [bp, rectf] = GenerateConnectionMouseHoverUiBlueprint(*connection);
					M2_LEVEL.SetMouseHoverUiPanel(std::make_unique<m2::UiPanelBlueprint>(bp), rectf);
					impl.currentMouseHoverLocation = *connection;
				}
			} else {
				// Remove mouse hover UI panel if activated
				if (impl.currentMouseHoverLocation) {
					M2_LEVEL.RemoveMouseHoverUiPanel();
					impl.currentMouseHoverLocation.reset();
				}
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
	M2_LEVEL.playerId = obj.GetId();

	return {};
}
m2::void_expected PlayerInitOtherInstance(m2::Object& obj) {
	DEBUG_FN();

	auto& chr = m2::AddCharacterToObject<m2g::ProxyEx::FastCharacterStorageIndex>(obj);

	// TODO check if the following is really necessary. If the TurnBasedServerUpdate is verified, it's necessary.
	// TODO Otherwise, we don't need to fill the character with cards and resources.

	chr.UnsafeSetVariable(m2g::pb::MONEY, m2::VariableValue{17});
	chr.UnsafeSetVariable(m2g::pb::INCOME_POINTS, m2::VariableValue{0});

	// Add industry tiles
	for (auto industry_tile = m2g::pb::COTTON_MILL_TILE_I;
		 industry_tile <= m2g::pb::MANUFACTURED_GOODS_TILE_VIII;
		 industry_tile = static_cast<m2g::pb::CardType>(m2::I(industry_tile) + 1)) {
		// Lookup possession count
		const auto& card = M2_GAME.GetCard(industry_tile);
		auto possession_limit = card.GetConstant(m2g::pb::POSSESSION_LIMIT).GetIntOrZero();
		m2Repeat(possession_limit) { chr.UnsafeAddCard(industry_tile); }
		 }

	// Add connection tiles
	const auto& road_card = M2_GAME.GetCard(m2g::pb::ROAD_TILE);
	auto road_possession_limit = road_card.GetConstant(m2g::pb::POSSESSION_LIMIT).GetIntOrZero();
	m2Repeat(road_possession_limit) { chr.UnsafeAddCard(m2g::pb::ROAD_TILE); }

	return {};
}

size_t PlayerCardCount(const m2::Character& player) {
	return player.CountCards(m2g::pb::CardCategory::CARD_CATEGORY_CITY_CARD)
	+ player.CountCards(m2g::pb::CardCategory::CARD_CATEGORY_WILD_CARD)
	+ player.CountCards(m2g::pb::CardCategory::CARD_CATEGORY_INDUSTRY_CARD);
}
std::vector<m2g::pb::CardType> PlayerCards(const m2::Character& player) {
	const auto& playerChr = dynamic_cast<const m2::FastCharacter&>(player);
	const auto cityCards = playerChr.GetCardTypes(m2g::pb::CARD_CATEGORY_CITY_CARD);
	const auto industryCards = playerChr.GetCardTypes(m2g::pb::CARD_CATEGORY_INDUSTRY_CARD);
	const auto wildCards = playerChr.GetCardTypes(m2g::pb::CARD_CATEGORY_WILD_CARD);
	auto cards = cityCards;
	cards.insert(cards.end(), industryCards.begin(), industryCards.end());
	cards.insert(cards.end(), wildCards.begin(), wildCards.end());
	return cards;
}

int PlayerLinkCount(const m2::Character& player) {
	auto road_characters = GetCharacterPool()
			| std::views::filter(m2::IsComponentOfAnyDescendant(player.OwnerId()))
			| std::views::filter(IsRoadCharacter);
	return std::accumulate(road_characters.begin(), road_characters.end(), 0, [](int acc, m2::Character& road_char) -> int {
		return acc + LinkCountOfRoadCharacter(road_char);
	});
}
int PlayerEstimatedVictoryPoints(const m2::Character& player) {
	auto soldFactories = GetCharacterPool()
			| std::views::filter(m2::IsComponentOfAnyDescendant(player.OwnerId()))
			| std::views::filter(IsFactoryCharacter)
			| std::views::filter(IsFactorySold);
	return std::accumulate(soldFactories.begin(), soldFactories.end(), 0, [](int acc, m2::Character& factoryCharacter) -> int {
		const auto& industryTileCard = M2_GAME.GetCard(ToIndustryTileOfFactoryCharacter(factoryCharacter));
		return acc + industryTileCard.GetConstant(m2g::pb::VICTORY_POINTS_BONUS).GetIntOrZero();
	});
}
int PlayerVictoryPoints(const m2::Character& player) {
	return player.GetVariable(m2g::pb::VICTORY_POINTS).GetIntOrZero();
}
int PlayerIncomePoints(const m2::Character& player) {
	return player.GetVariable(m2g::pb::INCOME_POINTS).GetIntOrZero();
}
int PlayerMoney(const m2::Character& player) {
	return player.GetVariable(m2g::pb::MONEY).GetIntOrZero();
}

size_t PlayerIndustryTileCount(const m2::Character& player) {
	return player.CountCards(m2g::pb::CardCategory::CARD_CATEGORY_COAL_MINE_TILE)
		+ player.CountCards(m2g::pb::CardCategory::CARD_CATEGORY_IRON_WORKS_TILE)
		+ player.CountCards(m2g::pb::CardCategory::CARD_CATEGORY_BREWERY_TILE)
		+ player.CountCards(m2g::pb::CardCategory::CARD_CATEGORY_COTTON_MILL_TILE)
		+ player.CountCards(m2g::pb::CardCategory::CARD_CATEGORY_MANUFACTURED_GOODS_TILE)
		+ player.CountCards(m2g::pb::CardCategory::CARD_CATEGORY_POTTERY_TILE);
}
std::optional<m2g::pb::CardType> PlayerNextIndustryTileOfCategory(const m2::Character& player, const m2g::pb::CardCategory tile_category) {
	// Find the card with the category with the smallest integer value
	auto tiles = dynamic_cast<const m2::FastCharacter&>(player).GetCardTypes(tile_category);
	if (tiles.empty()) {
		return std::nullopt;
	}
	std::ranges::sort(tiles);
	return tiles[0];
}
std::optional<m2g::pb::CardType> PlayerNextIndustryTileOfIndustry(const m2::Character& player, const Industry industry) {
	return PlayerNextIndustryTileOfCategory(player, industry_tile_category_of_industry(industry));
}
size_t PlayerBuiltFactoryCount(const m2::Character& player) {
	auto factories_view = GetCharacterPool()
		| std::views::filter(m2::IsComponentOfAnyDescendant(player.OwnerId()))
		| std::views::filter(IsFactoryCharacter);
	return std::distance(factories_view.begin(), factories_view.end());
}
std::set<IndustryLocation> PlayerBuiltFactoryLocations(const m2::Character& player) {
	auto factories_view = GetCharacterPool()
		| std::views::filter(m2::IsComponentOfAnyDescendant(player.OwnerId()))
		| std::views::filter(IsFactoryCharacter)
		| std::views::transform(ToIndustryLocationOfFactoryCharacter);
	return {factories_view.begin(), factories_view.end()};
}
std::set<IndustryLocation> PlayerSellableFactoryLocations(const m2::Character& player) {
	auto factories_view = GetCharacterPool()
		| std::views::filter(m2::IsComponentOfAnyDescendant(player.OwnerId()))
		| std::views::filter(IsFactoryCharacter)
		| std::views::filter(IsFactoryNotSold)
		| std::views::filter([](const m2::Character& c) {
			return is_sellable_industry(ToIndustryOfFactoryCharacter(c));
		})
		| std::views::transform(ToIndustryLocationOfFactoryCharacter);
	return {factories_view.begin(), factories_view.end()};
}
m2::void_expected PlayerCanOverbuild(const m2::Character& player, const IndustryLocation location, const m2g::pb::CardType card) {
	// Check the industry type of the already built factory
	const auto* factory = FindFactoryAtLocation(location);
	const auto industryOfFactory = ToIndustryOfFactoryCharacter(factory->GetCharacter());
	const auto cityOfFactory = ToCityOfFactoryCharacter(factory->GetCharacter());

	// Check if the selected card can build the same industry
	if (card == m2g::pb::WILD_LOCATION_CARD || card == m2g::pb::WILD_INDUSTRY_CARD || card == cityOfFactory || card == industryOfFactory) {
		// Can build
	} else {
		return m2::make_unexpected("Selected card cannot overbuild the same industry type");
	}

	// Check if the factory belongs to the player
	if (player.OwnerId() == factory->GetParentId()) {
		// Any industry can be overbuilt
		return {};
	}

	// Only coal mine or iron works can be overbuilt
	if (industryOfFactory != m2g::pb::COAL_MINE_CARD && industryOfFactory != m2g::pb::IRON_WORKS_CARD) {
		return m2::make_unexpected("Selected industry type cannot be overbuilt when it belongs to another player");
	}
	// There must be no resources left on the whole board of the type of the resource of the factory
	if (industryOfFactory == m2g::pb::COAL_MINE_CARD) {
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

std::set<m2g::pb::CardType> PlayerCitiesInNetwork(const m2::Character& player) {
	std::set<m2g::pb::CardType> cities;

	auto cities_view = GetCharacterPool()
		| std::views::filter(m2::IsComponentOfAnyDescendant(player.OwnerId()))
		| std::views::filter(IsFactoryCharacter)
		| std::views::transform(ToCityOfFactoryCharacter);
	cities.insert(cities_view.begin(), cities_view.end());

	auto roads_view = GetCharacterPool()
		| std::views::filter(m2::IsComponentOfAnyDescendant(player.OwnerId()))
		| std::views::filter(IsRoadCharacter)
		| std::views::transform(ToCitiesOfRoadCharacter);
	for (const auto& road_cities : roads_view) {
		std::ranges::copy(road_cities, std::inserter(cities, cities.begin()));
	}

	return cities;
}
std::set<m2g::pb::SpriteType> PlayerCanalsInNetwork(const m2::Character& player, Connection provisional_extra_connection) {
	std::set<m2g::pb::SpriteType> canals;

	auto cities_in_network = PlayerCitiesInNetwork(player);
	if (provisional_extra_connection) {
		auto extra_cities = cities_from_connection(provisional_extra_connection);
		cities_in_network.insert(extra_cities.begin(), extra_cities.end());
	}

	std::ranges::for_each(cities_in_network, [&canals](m2g::pb::CardType city) {
		// Iterate and find all the canals that have the city as one of it's legs
		for (int i = m2g::pb::BELPER_DERBY_CANAL_RAILROAD; i <= m2g::pb::REDDITCH_OXFORD_CANAL_RAILROAD; ++i) {
			auto road_location_type = static_cast<m2g::pb::SpriteType>(i);
			const auto& road_location = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(road_location_type));
			if (std::ranges::any_of(road_location.Cards(), is_canal_license) &&
				std::ranges::count(road_location.Cards(), city)) {
				canals.insert(road_location_type);
			}
		}
	});

	return canals;
}
std::set<m2g::pb::SpriteType> PlayerRailroadsInNetwork(const m2::Character& player, Connection provisional_extra_connection) {
	std::set<m2g::pb::SpriteType> railroads;

	auto cities_in_network = PlayerCitiesInNetwork(player);
	if (provisional_extra_connection) {
		auto extra_cities = cities_from_connection(provisional_extra_connection);
		cities_in_network.insert(extra_cities.begin(), extra_cities.end());
	}

	std::ranges::for_each(cities_in_network, [&railroads](m2g::pb::CardType city) {
		// Iterate and find all the railroads that have the city as one of it's legs
		for (int i = m2g::pb::BELPER_DERBY_CANAL_RAILROAD; i <= m2g::pb::REDDITCH_OXFORD_CANAL_RAILROAD; ++i) {
			auto road_location_type = static_cast<m2g::pb::SpriteType>(i);
			const auto& road_location = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(road_location_type));
			if (std::ranges::any_of(road_location.Cards(), is_railroad_license) &&
				std::ranges::count(road_location.Cards(), city)) {
				railroads.insert(road_location_type);
			}
		}
	});

	return railroads;
}
std::set<m2g::pb::SpriteType> PlayerConnectionsInNetwork(const m2::Character& player, Connection provisional_extra_connection) {
	if (M2G_PROXY.is_canal_era()) {
		return PlayerCanalsInNetwork(player, provisional_extra_connection);
	} else {
		return PlayerRailroadsInNetwork(player, provisional_extra_connection);
	}
}
