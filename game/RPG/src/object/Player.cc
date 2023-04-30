#include <rpg/object/Player.h>
#include <m2/Object.h>
#include "m2/Game.h"
#include "m2/Controls.h"
#include <rpg/object/ExplosiveWeapon.h>
#include <rpg/object/RangedWeapon.h>
#include <rpg/object/MeleeWeapon.h>
#include <m2/box2d/Detail.h>
#include <m2/M2.h>
#include <Item.pb.h>

rpg::Player::Player(m2::Object& obj) :
	animation_fsm(m2g::pb::ANIMATION_TYPE_PLAYER_MOVEMENT, obj.graphic_id()) {}

// Mouse primary button: shoot projectile (player can at most carry 3 primary weapons)
// Mouse secondary button: melee weapon (player can only carry one melee weapon)
// Mouse middle button: explosive weapon (player can only carry one explosive weapon)
// Mouse middle scroll: change primary projectile weapon
// Double tap directional buttons to dodge

m2::VoidValue rpg::Player::init(m2::Object& obj) {
	auto id = obj.id();
	auto main_sprite_type = GAME.level_editor_object_sprites[m2g::pb::PLAYER];

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.set_allow_sleep(false);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(GAME.get_sprite(main_sprite_type).background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ON_BACKGROUND);
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(GAME.get_sprite(main_sprite_type).foreground_collider_circ_radius_m());
	bp.mutable_foreground_fixture()->mutable_circ()->mutable_center_offset()->set_x(GAME.get_sprite(main_sprite_type).foreground_collider_center_offset_m().x);
	bp.mutable_foreground_fixture()->mutable_circ()->mutable_center_offset()->set_y(GAME.get_sprite(main_sprite_type).foreground_collider_center_offset_m().y);
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_ON_FOREGROUND);
	bp.set_mass(80.0f);
	bp.set_linear_damping(100.0f);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*LEVEL.world, obj.physique_id(), obj.position, bp);

	auto& gfx = obj.add_graphic(GAME.get_sprite(main_sprite_type));

	auto& chr = obj.add_full_character();
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_DASH_2S));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_MACHINE_GUN));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_SWORD));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_EXPLOSIVE));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_DASH_ENERGY));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_RANGED_ENERGY));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_MELEE_ENERGY));
	chr.add_resource(m2g::pb::RESOURCE_HP, 1.0f);

	obj.impl = std::make_unique<rpg::Player>(obj);

	phy.pre_step = [&, id=id](m2::Physique& phy) {
		auto* impl = dynamic_cast<rpg::Player*>(obj.impl.get());
		auto to_mouse = (GAME.mousePositionWRTGameWorld_m - obj.position).normalize();

		// TODO Use CharacterMovement instead
		m2::Vec2f moveDirection;
		if (GAME.events.is_key_down(m2::Key::UP)) {
			moveDirection.y += -1.0f;
			impl->animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_WALKUP});
		}
		if (GAME.events.is_key_down(m2::Key::DOWN)) {
			moveDirection.y += 1.0f;
			impl->animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_WALKDOWN});
		}
		if (GAME.events.is_key_down(m2::Key::LEFT)) {
			moveDirection.x += -1.0f;
			impl->animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_WALKLEFT});
		}
		if (GAME.events.is_key_down(m2::Key::RIGHT)) {
			moveDirection.x += 1.0f;
			impl->animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_WALKRIGHT});
		}
		float force;
		if (GAME.events.pop_key_press(m2::Key::DASH) && obj.character().use_item(obj.character().find_items(m2g::pb::ITEM_REUSABLE_DASH_2S))) {
			moveDirection = to_mouse;
			force = 100000.0f;
		} else {
			force = 2800.0f;
		}
		phy.body->ApplyForceToCenter(static_cast<b2Vec2>(moveDirection.normalize() * (GAME.deltaTime_s * force * 1000)), true);

		if (GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY) && obj.character().use_item(obj.character().find_items(m2g::pb::ITEM_REUSABLE_MACHINE_GUN))) {
			// New projectile
			auto& projectile = m2::create_object(obj.position, id).first;
			rpg::create_ranged_weapon_object(projectile, to_mouse, *GAME.get_item(m2g::pb::ITEM_REUSABLE_MACHINE_GUN));
			// Knock-back
			phy.body->ApplyForceToCenter(static_cast<b2Vec2>(m2::Vec2f::from_angle(to_mouse.angle_rads() + m2::PI) * 500.0f), true);
		}
		if (GAME.events.is_mouse_button_down(m2::MouseButton::SECONDARY) && obj.character().use_item(obj.character().find_items(m2g::pb::ITEM_REUSABLE_SWORD))) {
			auto& melee = m2::create_object(obj.position, id).first;
			rpg::create_melee_object(melee, to_mouse, *GAME.get_item(m2g::pb::ITEM_REUSABLE_SWORD), true);
		}
		if (GAME.events.is_mouse_button_down(m2::MouseButton::MIDDLE) && obj.character().use_item(obj.character().find_items(m2g::pb::ITEM_REUSABLE_EXPLOSIVE))) {
			auto& explosive = m2::create_object(obj.position, id).first;
			rpg::create_explosive_object(explosive, to_mouse, *GAME.get_item(m2g::pb::ITEM_REUSABLE_EXPLOSIVE));
		}
	};
	phy.on_collision = [&phy, &chr](MAYBE m2::Physique& me, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (other.parent().character_id() && 10.0f < m2::Vec2f{phy.body->GetLinearVelocity()}.length()) {
			auto& other_char = other.parent().character();
			m2::Character::execute_interaction(chr, m2g::pb::InteractionType::STUN, other_char, m2g::pb::InteractionType::GET_STUNNED_BY);
		}
	};
	phy.post_step = [&obj](m2::Physique& phy) {
		auto* impl = dynamic_cast<rpg::Player*>(obj.impl.get());
		// We must call time before other signals
		impl->animation_fsm.time(GAME.deltaTime_s);
		if (m2::Vec2f(phy.body->GetLinearVelocity()).is_small(0.5f)) {
			impl->animation_fsm.signal(m2::AnimationFsmSignal{m2g::pb::ANIMATION_STATE_IDLE});
		}
	};
	gfx.pre_draw = [&](m2::Graphic& gfx) {
		gfx.draw_effect_health_bar = chr.get_resource(m2g::pb::RESOURCE_HP);
	};

	LEVEL.playerId = LEVEL.objects.get_id(&obj);
	return {};
}
