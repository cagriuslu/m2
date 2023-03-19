#include <rpg/object/DroppedItem.h>
#include <m2g/Object.h>
#include <Item.pb.h>
#include <m2/box2d/Utils.h>
#include <m2/Game.h>
#include <rpg/object/Player.h>

m2::VoidValue obj::create_dropped_item(m2::Object &obj, m2g::pb::ItemType item_type) {
	const auto& sprite = GAME.get_sprite(GAME.get_item(item_type)->game_sprite());

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

	phy.on_collision = [=](m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		other.parent().character().add_item(GAME.get_item(item_type));
		GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
	};

	return {};
}
