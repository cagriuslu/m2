#include <rpg/object/ConsumableDrop.h>
#include <m2g/SpriteBlueprint.h>
#include <m2/box2d/Utils.h>
#include <m2/Game.hh>
#include <rpg/object/Player.h>

obj::ConsumableDrop::ConsumableDrop(const itm::ConsumableBlueprint &blueprint) : blueprint(blueprint) {}

m2::VoidValue obj::ConsumableDrop::init(m2::Object &obj, const itm::ConsumableBlueprint &blueprint, m2::Vec2f pos) {
	obj = m2::Object{pos};

	auto& phy = obj.add_physique();

	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::STATIC);
	bp.mutable_circ()->set_radius(GAME.lookup_sprite(blueprint.drop_sprite).collider_circ_radius_m());
	bp.set_allow_sleep(true);
	bp.set_is_bullet(false);
	bp.set_is_sensor(true);
	bp.set_category(m2::pb::BodyCategory::FRIEND_PICKUP);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), pos, bp);

	auto& gfx = obj.add_graphic(GAME.lookup_sprite(blueprint.drop_sprite));

	obj.impl = std::make_unique<ConsumableDrop>(blueprint);

	phy.on_collision = [&obj](m2::comp::Physique& phy, m2::comp::Physique& other) {
		auto* impl = dynamic_cast<ConsumableDrop*>(obj.impl.get());
		dynamic_cast<Player*>(other.parent().impl.get())->add_consumable(impl->blueprint);
		GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
	};

	return {};
}
