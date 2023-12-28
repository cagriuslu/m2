#include <rpg/Objects.h>
#include <Item.pb.h>
#include <m2/box2d/Detail.h>
#include <m2/Game.h>

m2::void_expected rpg::create_dropped_item(m2::Object &obj, m2g::pb::ItemType item_type) {
	const auto& sprite = GAME.get_sprite(GAME.get_named_item(item_type).ui_sprite());

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

	phy.on_collision = [item_type](m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (auto* other_char = other.parent().get_character(); other_char) {
			m2g::pb::InteractionData data;
			data.set_item_type(item_type);
			other_char->execute_interaction(data);
		}
		GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
	};

	return {};
}
