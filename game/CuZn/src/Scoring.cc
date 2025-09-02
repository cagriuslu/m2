#include <cuzn/Scoring.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/Road.h>
#include <cuzn/object/Factory.h>
#include <m2g_ResourceType.pb.h>
#include <m2/Game.h>
#include <m2/Log.h>

using namespace m2g;

void score_links_and_remove_roads() {
	std::ranges::for_each(
			M2G_PROXY.multiPlayerObjectIds
			| std::views::transform(m2::to_object_of_id)
			| std::views::transform(m2::to_character_of_object),
			[](m2::Character& player) {
				const auto linkCount = PlayerLinkCount(player);
				LOG_INFO("Player link count", linkCount);
				player.AddResource(pb::VICTORY_POINTS, m2::ToFloat(linkCount));
			});
	RemoveAllRoads();
}

void score_sold_factories_and_remove_obsolete() {
	std::ranges::for_each(
			M2_LEVEL.characters
				| std::views::transform(m2::ToCharacterBase)
				| std::views::filter(IsFactoryCharacter)
				| std::views::filter(IsFactorySold),
			[](const m2::Character& factory) {
				auto& player = factory.Owner().TryGetParent()->GetCharacter();
				const auto& industryTileItem = M2_GAME.GetNamedItem(ToIndustryTileOfFactoryCharacter(factory));
				const auto victoryPointsBonus = industryTileItem.GetAttribute(pb::VICTORY_POINTS_BONUS);
				LOG_INFO("Player victory points bonus", victoryPointsBonus);
				player.AddResource(pb::VICTORY_POINTS, victoryPointsBonus);
			});
	RemoveObsoleteFactories();
}
