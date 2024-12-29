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
			M2G_PROXY.multi_player_object_ids
			| std::views::transform(m2::to_object_of_id)
			| std::views::transform(m2::to_character_of_object),
			[](m2::Character& player) {
				const auto linkCount = player_link_count(player);
				LOG_INFO("Player link count", linkCount);
				player.add_resource(pb::VICTORY_POINTS, m2::F(linkCount));
			});
	remove_all_roads();
}

void score_sold_factories_and_remove_obsolete() {
	std::ranges::for_each(
			M2_LEVEL.characters
				| std::views::transform(m2::to_character_base)
				| std::views::filter(is_factory_character)
				| std::views::filter(is_factory_sold),
			[](const m2::Character& factory) {
				auto& player = factory.owner().get_parent()->character();
				const auto& industryTileItem = M2_GAME.GetNamedItem(to_industry_tile_of_factory_character(factory));
				const auto victoryPointsBonus = industryTileItem.get_attribute(pb::VICTORY_POINTS_BONUS);
				LOG_INFO("Player victory points bonus", victoryPointsBonus);
				player.add_resource(pb::VICTORY_POINTS, victoryPointsBonus);
			});
	remove_obsolete_factories();
}
