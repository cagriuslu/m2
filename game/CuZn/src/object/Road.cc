#include <cuzn/object/Road.h>
#include <m2/Game.h>
#include "m2/Log.h"
#include <cuzn/object/Factory.h>
#include <numeric>

namespace {
	int LinkCountOfCities(const std::set<City>& cities) {
		return std::accumulate(cities.begin(), cities.end(), 0, [](const int acc, const City city) -> int {
				// If the city is a merchant city, add 2 links
				if (is_merchant_city(city)) {
					return acc + 2;
				}
				// Else, iterate over locations in the city
				auto locations = industry_locations_in_city(city);
				return acc + std::accumulate(locations.begin(), locations.end(), 0, [](const int acc2, const IndustryLocation location) -> int {
					// Check if there's a built factory
					if (const auto* factory = FindFactoryAtLocation(location); factory && IsFactorySold(factory->character())) {
						const auto industry_tile = ToIndustryTileOfFactoryCharacter(factory->character());
						return acc2 + m2::iround(M2_GAME.GetNamedItem(industry_tile).GetAttribute(m2g::pb::LINK_BONUS));
					}
					return acc2;
				}); });
	}
}

m2::Object* FindRoadAtLocation(const m2g::pb::SpriteType location) {
	auto roads = M2_LEVEL.characters
				 | std::views::transform(m2::ToCharacterBase)
				 | std::views::filter(IsRoadCharacter)
				 | std::views::transform(m2::to_owner_of_component)
				 | std::views::filter(m2::is_object_in_area(std::get<m2::RectF>( M2G_PROXY.connection_positions[location])));
	if (auto road_it = roads.begin(); road_it != roads.end()) {
		return &*road_it;
	}
	return nullptr;
}

int LinkCountOfConnectionLocation(const Connection conn) {
	const auto cities = cities_from_connection(conn);
	return LinkCountOfCities({cities.begin(), cities.end()});
}

int LinkCountOfRoadCharacter(const m2::Character& chr) {
	if (not IsRoadCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to canal or railroad");
	}
	return LinkCountOfCities(ToCitiesOfRoadCharacter(chr));
}

void RemoveAllRoads() {
	std::vector<m2::ObjectId> ids;
	std::ranges::copy(
			M2_LEVEL.characters
				| std::views::transform(m2::ToCharacterBase)
				| std::views::filter(IsRoadCharacter)
				| std::views::transform(m2::to_owner_id_of_component),
			std::back_inserter(ids));

	// Delete objects immediately
	std::ranges::for_each(ids, [](m2::ObjectId id) {
		M2_LEVEL.objects.Free(id);
	});
}

m2::void_expected InitRoad(m2::Object& obj, const Connection connection) {
	DEBUG_FN();

	if (not is_connection(connection)) {
		throw M2_ERROR("Invalid connection");
	}

	// Add the city cards to the character
	auto& chr = obj.add_full_character();
	for (const auto city : cities_from_connection(connection)) {
		chr.AddNamedItem(M2_GAME.GetNamedItem(city));
	}

	const auto parent_id = obj.parent_id();
	const auto parent_index = M2G_PROXY.player_index(parent_id);
	const auto color = M2G_PROXY.player_colors[parent_index];
	auto& _gfx = obj.add_graphic(M2G_PROXY.is_canal_era() ? m2g::pb::SPRITE_CANAL : m2g::pb::SPRITE_RAILROAD);
	_gfx.onDraw = [color](m2::Graphic& gfx) {
		const auto connection_position = gfx.owner().position;
		const auto cell_rect = m2::RectF{connection_position - 0.75f, 1.5f, 1.5f};

		// Draw background with player's color
		const auto background_color =
				(M2_LEVEL.DimmingExceptions() && not M2_LEVEL.DimmingExceptions()->contains(gfx.owner_id()))
				? color * M2G_PROXY.dimming_factor : color;
		m2::Graphic::ColorRect(cell_rect, background_color);

		m2::Graphic::DefaultDrawCallback(gfx); // Draw connection
	};

	return {};
}
