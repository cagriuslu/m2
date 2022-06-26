#include <rpg/object/ConsumableDrop.h>
#include <m2g/SpriteBlueprint.h>
#include <m2/box2d/Utils.h>
#include <m2/Game.hh>
#include <rpg/object/Player.h>

obj::ConsumableDrop::ConsumableDrop(const itm::ConsumableBlueprint &blueprint) : blueprint(blueprint) {}

m2::VoidValue obj::ConsumableDrop::init(m2::Object &obj, const itm::ConsumableBlueprint &blueprint, m2::Vec2f pos) {
	obj = m2::Object{pos};

	auto& phy = obj.add_physique();
	phy.body = m2::box2d::create_body(
			*GAME.world,
			obj.physique_id(),
			true,
			false,
			pos,
			true,
			false,
			true,
			m2::box2d::CAT_PLAYER_ITEM,
			0,
			{},
			{},
			0.0f,
			std::get<m2::ColliderBlueprint::Circle>(m2g::sprites[blueprint.drop_sprite_index].collider.variant).radius_m,
			1.0f,
			1.0f,
			true
	);

	auto& gfx = obj.add_graphic();
	gfx.textureRect = m2g::sprites[blueprint.drop_sprite_index].texture_rect;

	obj.impl = std::make_unique<ConsumableDrop>(blueprint);

	phy.on_collision = [&obj](m2::comp::Physique& phy, m2::comp::Physique& other) {
		auto* impl = dynamic_cast<ConsumableDrop*>(obj.impl.get());
		dynamic_cast<Player*>(other.parent().impl.get())->add_consumable(impl->blueprint);
		GAME.add_deferred_action(m2::create_object_deleter(phy.object_id));
	};

	return {};
}
