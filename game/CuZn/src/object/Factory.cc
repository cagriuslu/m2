#include <cuzn/object/Factory.h>
#include <m2/Game.h>
#include <cuzn/detail/Graphic.h>
#include <cuzn/detail/Income.h>
#include <m2/Log.h>

using namespace m2g;
using namespace m2g::pb;

m2::Object* FindFactoryAtLocation(Location location) {
	auto factories = M2_LEVEL.characters
		| std::views::transform(m2::ToCharacterBase)
		| std::views::filter(IsFactoryCharacter)
		| std::views::transform(m2::ToOwner)
		| std::views::filter(m2::is_object_in_area(std::get<m2::RectF>(M2G_PROXY.industry_positions[location])));
	if (auto factory_it = factories.begin(); factory_it != factories.end()) {
		return &*factory_it;
	}
	return nullptr;
}

int RequiredBeerCountToSell(IndustryLocation location) {
	if (auto* factory = FindFactoryAtLocation(location); not factory) {
		throw M2_ERROR("Invalid factory location");
	} else {
		auto industry_tile = ToIndustryTileOfFactoryCharacter(factory->GetCharacter());
		return m2::RoundI(M2_GAME.GetNamedItem(industry_tile).GetAttribute(BEER_COST));
	}
}

void RemoveObsoleteFactories() {
	std::vector<m2::ObjectId> ids;
	ids.reserve(20); // Reserve an average amount of space
	std::ranges::copy(
		M2_LEVEL.characters
		| std::views::transform(m2::ToCharacterBase)
		| std::views::filter(IsFactoryCharacter)
		| std::views::filter(IsFactoryLevel1)
		| std::views::transform(m2::ToOwnerId),
		std::back_inserter(ids));

	// Delete objects immediately
	std::ranges::for_each(ids, [](m2::ObjectId id) {
		M2_LEVEL.objects.Free(id);
	});
}

void FlipExhaustedFactories() {
	std::ranges::for_each(
		M2_LEVEL.characters
			| std::views::transform(m2::ToCharacterBase)
			| std::views::filter(IsFactoryCharacter)
			| std::views::filter(IsFactoryNotSold),
		[](m2::Character& chr) {
			const auto is_coal_mine_exhausted = ToIndustryOfFactoryCharacter(chr) == COAL_MINE_CARD
				&& m2::IsEqual(chr.GetResource(COAL_CUBE_COUNT), 0.0f, 0.001f);
			const auto is_iron_works_exhausted = ToIndustryOfFactoryCharacter(chr) == IRON_WORKS_CARD
				&& m2::IsEqual(chr.GetResource(IRON_CUBE_COUNT), 0.0f, 0.001f);
			const auto is_brewery_exhausted = ToIndustryOfFactoryCharacter(chr) == BREWERY_CARD
				&& m2::IsEqual(chr.GetResource(BEER_BARREL_COUNT), 0.0f, 0.001f);
			if (is_coal_mine_exhausted || is_iron_works_exhausted || is_brewery_exhausted) {
				SellFactory(chr);
			}
		});
}

void SellFactory(m2::Character& factory_chr) {
	const auto tileType = ToIndustryTileOfFactoryCharacter(factory_chr);
	const auto& tileTtem = M2_GAME.GetNamedItem(tileType);
	// Earn income points
	const auto incomeBonus = m2::RoundI(tileTtem.GetAttribute(INCOME_POINTS_BONUS));
	const auto currIncomePoints = m2::RoundI(factory_chr.Owner().TryGetParent()->GetCharacter().GetAttribute(INCOME_POINTS));
	factory_chr.Owner().TryGetParent()->GetCharacter().SetAttribute(INCOME_POINTS, m2::F(ClampIncomePoints(currIncomePoints + incomeBonus)));
	// Flip the tile
	factory_chr.SetResource(IS_SOLD, 1.0f);
}

bool IsFactorySold(m2::Character& chr) {
	if (not IsFactoryCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return m2::IsEqual(chr.GetResource(m2g::pb::IS_SOLD), 1.0f, 0.001f);
}

bool IsFactoryLevel1(m2::Character& chr) {
	static std::set<IndustryTile> level_1_tiles{COTTON_MILL_TILE_I, IRON_WORKS_TILE_I, BREWERY_TILE_I, COAL_MINE_TILE_I,
		POTTERY_TILE_I, MANUFACTURED_GOODS_TILE_I};
	return level_1_tiles.contains(ToIndustryTileOfFactoryCharacter(chr));
}

City ToCityOfFactoryCharacter(m2::Character& chr) {
	if (not IsFactoryCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return chr.FindItems(m2g::pb::ITEM_CATEGORY_CITY_CARD)->Type();
}

Industry ToIndustryOfFactoryCharacter(const m2::Character& chr) {
	if (not IsFactoryCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return chr.FindItems(m2g::pb::ITEM_CATEGORY_INDUSTRY_CARD)->Type();
}

IndustryTile ToIndustryTileOfFactoryCharacter(const m2::Character& chr) {
	if (not IsFactoryCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	auto industry = ToIndustryOfFactoryCharacter(chr);
	auto industry_tile_category = industry_tile_category_of_industry(industry);
	return chr.FindItems(industry_tile_category)->Type();
}

IndustryLocation ToIndustryLocationOfFactoryCharacter(m2::Character& chr) {
	if (not IsFactoryCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return *industry_location_on_position(chr.Owner().position);
}

m2::void_expected InitFactory(m2::Object& obj, City city, IndustryTile industry_tile) {
	DEBUG_FN();

	if (not is_city(city)) {
		throw M2_ERROR("Invalid city");
	}
	if (not is_industry_tile(industry_tile)) {
		throw M2_ERROR("Invalid industry tile");
	}

	auto parent_id = obj.GetParentId();
	auto parent_index = M2G_PROXY.player_index(parent_id);
	auto industry = industry_of_industry_tile(industry_tile);

	// Add all available information to the factories: industry, city, industry tile
	auto& chr = obj.AddFullCharacter();
	chr.AddNamedItem(M2_GAME.GetNamedItem(industry));
	chr.AddNamedItem(M2_GAME.GetNamedItem(city));
	chr.AddNamedItem(M2_GAME.GetNamedItem(industry_tile));

	auto color = M2G_PROXY.player_colors[parent_index];
	auto& _gfx = obj.AddGraphic(industry_sprite_of_industry(industry));
	_gfx.onDraw = [color](m2::Graphic& gfx) {
		auto top_left_cell_pos = gfx.Owner().position;
		auto cell_rect = m2::RectF{top_left_cell_pos - 0.5f, 2.0f, 2.0f};

		// Draw background with player's color
		auto background_color = (M2_LEVEL.DimmingExceptions() && not M2_LEVEL.DimmingExceptions()->contains(gfx.OwnerId()))
			? color * M2G_PROXY.dimming_factor : color;
		m2::Graphic::ColorRect(cell_rect, background_color);

		// If sold, draw the black bottom half
		if (m2::IsOne(gfx.Owner().GetCharacter().GetResource(IS_SOLD), 0.005f)) {
			const auto bottom_half_cell_rect = m2::RectF{top_left_cell_pos.x - 0.5f, top_left_cell_pos.y + 0.5f, 2.0f, 1.0f};
			m2::Graphic::ColorRect(bottom_half_cell_rect, m2::RGB{0, 0, 0});
		}

		// Draw the industry graphic
		m2::Graphic::DefaultDrawCallback(gfx);

		// Draw the resources
		DrawResources(gfx.Owner().GetCharacter());
	};

	return {};
}
