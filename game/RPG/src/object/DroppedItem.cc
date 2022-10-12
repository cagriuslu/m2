#include <rpg/object/DroppedItem.h>
#include <m2g/Object.h>
#include <Item.pb.h>
#include <m2/box2d/Utils.h>
#include <m2/Game.hh>
#include <rpg/object/Player.h>

obj::DroppedItem::DroppedItem(m2g::pb::ItemType item_type) : item_type(item_type) {}

m2::VoidValue obj::create_dropped_item(m2::Object &obj, m2g::pb::ItemType item_type) {
	const auto& sprite = GAME.sprites[GAME.items[item_type].game_sprite()];

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::STATIC);
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(sprite.background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_is_sensor(true);
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_PICKUP);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), obj.position, bp);

	auto& gfx = obj.add_graphic(sprite);

	phy.on_collision = [&obj](m2::comp::Physique& phy, m2::comp::Physique& other) {
		auto* impl = dynamic_cast<DroppedItem*>(obj.impl.get());
		dynamic_cast<Player*>(other.parent().impl.get())->add_item(impl->item_type);
		GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
	};

	obj.impl = std::make_unique<DroppedItem>(item_type);

	return {};
}
