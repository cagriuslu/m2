#include <cuzn/Scoring.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/Road.h>
#include <cuzn/object/Factory.h>
#include <m2/Game.h>
#include <m2/Log.h>

using namespace m2g;

void score_links_and_remove_roads() {
	std::ranges::for_each(
			M2_LEVEL.multiPlayerObjectIds
			| std::views::transform(m2::ObjectIdToObject)
			| std::views::transform(m2::ObjectToCharacter),
			[](m2::Character& player) {
				const auto linkCount = PlayerLinkCount(player);
				LOG_INFO("Player link count", linkCount);
				player.UnsafeSetVariable(pb::VICTORY_POINTS, m2::VariableValue{player.GetVariable(pb::VICTORY_POINTS).GetIntOrZero() + linkCount});
			});
	RemoveAllRoads();
}

void score_sold_factories_and_remove_obsolete() {
	std::ranges::for_each(
			GetCharacterPool()
				| std::views::filter(IsFactoryCharacter)
				| std::views::filter(IsFactorySold),
			[](const m2::Character& factory) {
				auto& player = factory.Owner().TryGetParent()->GetCharacter();
				const auto& industryTileCard = M2_GAME.GetCard(ToIndustryTileOfFactoryCharacter(factory));
				const auto victoryPointsBonus = industryTileCard.GetConstant(pb::VICTORY_POINTS_BONUS).GetIntOrZero();
				LOG_INFO("Player victory points bonus", victoryPointsBonus);
				player.UnsafeSetVariable(pb::VICTORY_POINTS, m2::VariableValue{player.GetVariable(pb::VICTORY_POINTS).GetIntOrZero() + victoryPointsBonus});
			});
	RemoveObsoleteFactories();
}
