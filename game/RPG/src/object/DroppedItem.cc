#include <rpg/object/DroppedItem.h>
#include <Item.pb.h>
#include <m2/box2d/Detail.h>
#include <m2/Game.h>

m2::VoidValue rpg::create_dropped_item(m2::Object &obj, m2g::pb::ItemType item_type) {
	const auto& sprite = GAME.get_sprite(GAME.get_item(item_type)->ui_sprite());

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::STATIC);
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(sprite.background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_is_sensor(true);
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ITEM_ON_FOREGROUND);
	phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);

	obj.add_graphic(sprite);

	// Symbolic character just to execute interactions
	auto& chr = obj.add_tiny_character();

	phy.on_collision = [&chr](m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		m2::Character::execute_interaction(chr, other.parent().character(), m2g::pb::InteractionType::GIVE_ITEM);
		GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
	};
	chr.create_interaction = [item_type](MAYBE m2::Character& self, MAYBE m2::Character& other, m2g::pb::InteractionType type) -> std::optional<m2g::pb::InteractionData> {
		if (type == m2g::pb::InteractionType::GIVE_ITEM) {
			m2g::pb::InteractionData data;
			data.set_item_type(item_type);
			return data;
		}
		return std::nullopt;
	};

	return {};
}
