#include <rpg/object/Player.h>
#include <m2/Object.h>
#include <rpg/object/Enemy.h>
#include "m2/Game.hh"
#include "m2/Controls.h"
#include <rpg/object/ExplosiveWeapon.h>
#include <rpg/object/RangedWeapon.h>
#include <rpg/object/MeleeWeapon.h>
#include <m2/box2d/Utils.h>
#include <m2/M2.h>
#include <Item.pb.h>

obj::Player::Player(m2::Object& obj, const chr::CharacterBlueprint* blueprint) :
	char_state(blueprint), animation_fsm(blueprint->animation_type, obj.graphic_id()) {}

void obj::Player::add_item(m2g::pb::ItemType item_type) {
	items.push_back(item_type);
}

// Mouse primary button: shoot projectile (player can at most carry 3 primary weapons)
// Mouse secondary button: melee weapon (player can only carry one melee weapon)
// Mouse middle button: explosive weapon (player can only carry one explosive weapon)
// Mouse middle scroll: change primary projectile weapon
// Double tap directional buttons to dodge

m2::VoidValue obj::Player::init(m2::Object& obj, const chr::CharacterBlueprint* blueprint) {
	const auto id = obj.id();

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.set_allow_sleep(false);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(GAME.sprites[blueprint->main_sprite].background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ON_BACKGROUND);
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(GAME.sprites[blueprint->main_sprite].foreground_collider_circ_radius_m());
	bp.mutable_foreground_fixture()->mutable_circ()->mutable_center_offset()->set_x(GAME.sprites[blueprint->main_sprite].foreground_collider_center_offset_m().x);
	bp.mutable_foreground_fixture()->mutable_circ()->mutable_center_offset()->set_y(GAME.sprites[blueprint->main_sprite].foreground_collider_center_offset_m().y);
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ON_FOREGROUND);
	bp.set_mass(80.0f);
	bp.set_linear_damping(100.0f);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), obj.position, bp);

	auto& gfx = obj.add_graphic(GAME.sprites[blueprint->main_sprite]);

	auto& chr = obj.add_full_character();
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_DASH_2S));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_MACHINE_GUN));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_SWORD));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_GRENADE_LAUNCHER));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_DASH_ENERGY));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_RANGED_ENERGY));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_MELEE_ENERGY));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_EXPLOSIVE_ENERGY));
	chr.add_resource(m2g::pb::RESOURCE_HP, 1.0f);

	obj.impl = std::make_unique<obj::Player>(obj, blueprint);

	phy.pre_step = [&, id=id](m2::Physique& phy) {
		auto* impl = dynamic_cast<obj::Player*>(obj.impl.get());
		auto to_mouse = (GAME.mousePositionWRTGameWorld_m - obj.position).normalize();

		// TODO Use CharacterMovement instead
		m2::Vec2f moveDirection;
		if (GAME.events.is_key_down(m2::Key::UP)) {
			moveDirection.y += -1.0f;
			impl->animation_fsm.signal(m2g::pb::ANIMATION_STATE_WALKUP);
		}
		if (GAME.events.is_key_down(m2::Key::DOWN)) {
			moveDirection.y += 1.0f;
			impl->animation_fsm.signal(m2g::pb::ANIMATION_STATE_WALKDOWN);
		}
		if (GAME.events.is_key_down(m2::Key::LEFT)) {
			moveDirection.x += -1.0f;
			impl->animation_fsm.signal(m2g::pb::ANIMATION_STATE_WALKLEFT);
		}
		if (GAME.events.is_key_down(m2::Key::RIGHT)) {
			moveDirection.x += 1.0f;
			impl->animation_fsm.signal(m2g::pb::ANIMATION_STATE_WALKRIGHT);
		}
		float force;
		if (GAME.events.pop_key_press(m2::Key::DASH) && obj.character().use_item(obj.character().find_items(m2g::pb::ITEM_REUSABLE_DASH_2S))) {
			moveDirection = to_mouse;
			force = 100000.0f;
		} else {
			force = 2800.0f;
		}
		phy.body->ApplyForceToCenter(static_cast<b2Vec2>(moveDirection.normalize() * (GAME.deltaTicks_ms * force)), true);

		impl->char_state.process_time(GAME.deltaTime_s);

		if (GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY) && obj.character().use_item(obj.character().find_items(m2g::pb::ITEM_REUSABLE_MACHINE_GUN))) {
			// New projectile
			auto& projectile = m2::create_object(obj.position, id).first;
			rpg::create_ranged_weapon_object(projectile, to_mouse, GAME.get_item(m2g::pb::ITEM_REUSABLE_MACHINE_GUN));
			// Knock-back
			phy.body->ApplyForceToCenter(static_cast<b2Vec2>(m2::Vec2f::from_angle(to_mouse.angle_rads() + m2::PI) * 500.0f), true);
		}
		if (GAME.events.is_mouse_button_down(m2::MouseButton::SECONDARY) && obj.character().use_item(obj.character().find_items(m2g::pb::ITEM_REUSABLE_SWORD))) {
			auto& melee = m2::create_object(obj.position, id).first;
			rpg::create_melee_object(melee, to_mouse, GAME.get_item(m2g::pb::ITEM_REUSABLE_SWORD), true);
		}
		if (GAME.events.is_mouse_button_down(m2::MouseButton::MIDDLE) && obj.character().use_item(obj.character().find_items(m2g::pb::ITEM_REUSABLE_GRENADE_LAUNCHER))) {
			auto& explosive = m2::create_object(obj.position, id).first;
			rpg::create_explosive_object(explosive, to_mouse, GAME.get_item(m2g::pb::ITEM_REUSABLE_GRENADE_LAUNCHER));
		}
	};
	phy.on_collision = [&phy](MAYBE m2::Physique& me, m2::Physique& other) {
		auto* enemy_impl = dynamic_cast<obj::Enemy*>(other.parent().impl.get());
		if (enemy_impl && 10.0f < m2::Vec2f{phy.body->GetLinearVelocity()}.length()) {
			enemy_impl->stun();
		}
	};
	phy.post_step = [&obj](m2::Physique& phy) {
		auto* impl = dynamic_cast<obj::Player*>(obj.impl.get());
		// We must call time before other signals
		impl->animation_fsm.time(GAME.deltaTime_s);
		if (m2::Vec2f(phy.body->GetLinearVelocity()).is_small(0.5f)) {
			impl->animation_fsm.signal(m2g::pb::ANIMATION_STATE_IDLE);
		}
		// Consume consumables
		for (auto it = impl->items.begin(); it != impl->items.end(); ) {
			auto item = GAME.get_item(*it);
			if (item->usage() == m2::pb::Usage::CONSUMABLE) {
				for (const auto& resource : item->benefits()) {
					switch (resource.type()) {
						case m2g::pb::RESOURCE_HP:
							throw M2ERROR("Item pickup not implemented");
						default:
							break;
					}
				}
				it = impl->items.erase(it);
			} else {
				++it;
			}
		}
	};
	gfx.pre_draw = [&](m2::Graphic& gfx) {
		gfx.draw_effect_health_bar = chr.get_resource(m2g::pb::RESOURCE_HP);
	};

	GAME.playerId = GAME.objects.get_id(&obj);
	return {};
}
