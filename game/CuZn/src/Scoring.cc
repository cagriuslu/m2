#include <cuzn/Scoring.h>
#include <cuzn/object/HumanPlayer.h>
#include <cuzn/object/Road.h>
#include <cuzn/object/Factory.h>
#include <m2g_ResourceType.pb.h>
#include <m2/Game.h>

using namespace m2g;

void score_links_and_remove_roads() {
	std::ranges::for_each(
			M2G_PROXY.multi_player_object_ids
			| std::views::transform(m2::to_object_of_id)
			| std::views::transform(m2::to_character_of_object),
			[](m2::Character& player) {
				player.add_resource(pb::VICTORY_POINTS, m2::F(player_link_count(player)));
			});
	remove_all_roads();
}

void score_sold_factories_and_remove_obsolete() {
	std::ranges::for_each(M2_LEVEL.characters
						  | std::views::transform(m2::to_character_base)
						  | std::views::filter(is_factory_character)
						  | std::views::filter(is_factory_sold),
			[](m2::Character& factory) {
				auto& player = factory.owner();
				player.character().add_resource(pb::VICTORY_POINTS, factory.get_attribute(pb::VICTORY_POINTS_BONUS));
			});
	remove_obsolete_factories();
}
