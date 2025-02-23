#include <rpg/Objects.h>
#include <Item.pb.h>
#include <m2/box2d/Detail.h>
#include <m2/Game.h>

m2::void_expected rpg::create_dropped_item(m2::Object &obj, m2g::pb::ItemType item_type) {
	const auto& sprite = std::get<m2::Sprite>(M2_GAME.GetSpriteOrTextLabel(M2_GAME.GetNamedItem(item_type).ui_sprite()));

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::STATIC);
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(sprite.BackgroundColliderCircRadiusM());
	bp.mutable_background_fixture()->set_is_sensor(true);
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ITEM_ON_FOREGROUND);
	phy.body = m2::box2d::CreateBody(*M2_LEVEL.world, obj.physique_id(), obj.position, bp);

	obj.add_graphic(M2_GAME.GetNamedItem(item_type).ui_sprite());

	phy.onCollision = [item_type](m2::Physique& phy, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (auto* other_char = other.owner().get_character(); other_char) {
			m2g::pb::InteractionData data;
			data.set_item_type(item_type);
			other_char->ExecuteInteraction(data);
		}
		M2_DEFER(m2::create_object_deleter(phy.owner_id()));
	};

	return {};
}
