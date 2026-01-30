#include <cuzn/object/Factory.h>
#include <m2/Game.h>
#include <cuzn/detail/Graphic.h>
#include <cuzn/detail/Income.h>
#include <m2g_CardType.pb.h>
#include <m2/ObjectEx.h>
#include <m2/Log.h>

using namespace m2g;
using namespace m2g::pb;

m2::Object* FindFactoryAtLocation(Location location) {
	auto factories = GetCharacterPool()
		| std::views::filter(IsFactoryCharacter)
		| std::views::transform(m2::ToOwner)
		| std::views::filter(m2::IsObjectInArea(std::get<m2::RectF>(M2G_PROXY.industry_positions[location])));
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
		return M2_GAME.GetCard(industry_tile).GetConstant(BEER_COST).GetIntOrZero();
	}
}

void RemoveObsoleteFactories() {
	std::vector<m2::ObjectId> ids;
	ids.reserve(20); // Reserve an average amount of space
	std::ranges::copy(
		GetCharacterPool()
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
		GetCharacterPool()
			| std::views::filter(IsFactoryCharacter)
			| std::views::filter(IsFactoryNotSold),
		[](m2::Character& chr) {
			const auto is_coal_mine_exhausted = ToIndustryOfFactoryCharacter(chr) == COAL_MINE_CARD
				&& chr.GetVariable(COAL_CUBE_COUNT).GetIntOrZero() == 0;
			const auto is_iron_works_exhausted = ToIndustryOfFactoryCharacter(chr) == IRON_WORKS_CARD
				&& chr.GetVariable(IRON_CUBE_COUNT).GetIntOrZero() == 0;
			const auto is_brewery_exhausted = ToIndustryOfFactoryCharacter(chr) == BREWERY_CARD
				&& chr.GetVariable(BEER_BARREL_COUNT).GetIntOrZero() == 0;
			if (is_coal_mine_exhausted || is_iron_works_exhausted || is_brewery_exhausted) {
				SellFactory(chr);
			}
		});
}

void SellFactory(m2::Character& factory_chr) {
	const auto tileType = ToIndustryTileOfFactoryCharacter(factory_chr);
	const auto& tileTtem = M2_GAME.GetCard(tileType);
	// Earn income points
	const auto incomeBonus = tileTtem.GetConstant(INCOME_POINTS_BONUS).GetIntOrZero();
	const auto currIncomePoints = factory_chr.Owner().TryGetParent()->GetCharacter().GetVariable(INCOME_POINTS).GetIntOrZero();
	factory_chr.Owner().TryGetParent()->GetCharacter().UnsafeSetVariable(INCOME_POINTS, m2::IVFE{ClampIncomePoints(currIncomePoints + incomeBonus)});
	// Flip the tile
	factory_chr.UnsafeSetVariable(IS_SOLD, m2::IVFE{1});
}

bool IsFactorySold(m2::Character& chr) {
	if (not IsFactoryCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return chr.GetVariable(m2g::pb::IS_SOLD).GetIntOrZero() == 1;
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
	return *chr.GetFirstCardType(CARD_CATEGORY_CITY_CARD);
}

Industry ToIndustryOfFactoryCharacter(const m2::Character& chr) {
	if (not IsFactoryCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return *chr.GetFirstCardType(CARD_CATEGORY_INDUSTRY_CARD);
}

IndustryTile ToIndustryTileOfFactoryCharacter(const m2::Character& chr) {
	if (not IsFactoryCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	const auto industry = ToIndustryOfFactoryCharacter(chr);
	const auto industry_tile_category = industry_tile_category_of_industry(industry);
	return *chr.GetFirstCardType(industry_tile_category);
}

IndustryLocation ToIndustryLocationOfFactoryCharacter(m2::Character& chr) {
	if (not IsFactoryCharacter(chr)) {
		throw M2_ERROR("Character doesn't belong to a factory");
	}
	return *industry_location_on_position(chr.Owner().InferPositionF());
}

m2::void_expected InitFactory(m2::Object& obj, const m2::VecF& position, City city, IndustryTile industry_tile) {
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
	auto& chr = m2::AddCharacterToObject<m2g::ProxyEx::FastCharacterStorageIndex>(obj);
	chr.UnsafeAddCard(industry);
	chr.UnsafeAddCard(city);
	chr.UnsafeAddCard(industry_tile);

	auto color = M2G_PROXY.player_colors[parent_index];
	auto& _gfx = obj.AddGraphic(m2::pb::UprightGraphicsLayer::SEA_LEVEL_UPRIGHT, industry_sprite_of_industry(industry));
	_gfx.position = position;
	_gfx.onDraw = [color](m2::Graphic& gfx) {
		auto top_left_cell_pos = gfx.position;
		auto cell_rect = m2::RectF{top_left_cell_pos - 0.5f, 2.0f, 2.0f};

		// Draw background with player's color
		auto background_color = (M2_LEVEL.GetDimmingExceptions() && not M2_LEVEL.GetDimmingExceptions()->contains(gfx.OwnerId()))
			? color * M2G_PROXY.dimming_factor : color;
		m2::Graphic::ColorRect(cell_rect, background_color);

		// If sold, draw the black bottom half
		if (gfx.Owner().GetCharacter().GetVariable(IS_SOLD).GetIntOrZero() == 1) {
			const auto bottom_half_cell_rect = m2::RectF{top_left_cell_pos.GetX() - 0.5f, top_left_cell_pos.GetY() + 0.5f, 2.0f, 1.0f};
			m2::Graphic::ColorRect(bottom_half_cell_rect, m2::RGB{0, 0, 0});
		}

		// Draw the industry graphic
		m2::Graphic::DefaultDrawCallback(gfx);

		// Draw the resources
		DrawResources(gfx.Owner().GetCharacter());
	};

	return {};
}
