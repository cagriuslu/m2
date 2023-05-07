#include <rpg/object/Player.h>
#include <m2/Object.h>
#include "m2/Game.h"
#include <rpg/Detail.h>
#include "m2/Controls.h"
#include <rpg/Context.h>
#include <m2/game/CharacterMovement.h>
#include <rpg/object/RangedWeapon.h>
#include <rpg/object/MeleeWeapon.h>
#include <m2/box2d/Detail.h>
#include <m2/M2.h>
#include <Item.pb.h>

rpg::Player::Player(m2::Object& obj) : animation_fsm(m2g::pb::ANIMATION_TYPE_PLAYER_MOVEMENT, obj.graphic_id()) {}

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
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_GUN));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_REUSABLE_SWORD));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_DASH_ENERGY));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_RANGED_ENERGY));
	chr.add_item(GAME.get_item(m2g::pb::ITEM_AUTOMATIC_MELEE_ENERGY));
	chr.add_resource(m2g::pb::RESOURCE_HP, 1.0f);

	obj.impl = std::make_unique<rpg::Player>(obj);
	auto& impl = dynamic_cast<Player&>(*obj.impl);

	phy.pre_step = [&, id=id](m2::Physique& phy) {
		auto& chr = obj.character();
		auto vector_to_mouse = (GAME.mouse_position_world_m() - obj.position).normalize();

		m2::Vec2f move_vector;
		float move_force{};
		// Check if dash
		if (GAME.events.pop_key_press(m2::Key::DASH) && chr.use_item(chr.find_items(m2g::pb::ITEM_REUSABLE_DASH_2S))) {
			move_vector = vector_to_mouse;
			move_force = 100000000.0f;
		} else {
			// Character movement
			auto [direction_enum, direction_vector] = m2::calculate_character_movement(m2::Key::LEFT, m2::Key::RIGHT, m2::Key::UP, m2::Key::DOWN);
			auto anim_state_type = detail::to_animation_state_type(direction_enum);
			impl.animation_fsm.signal(m2::AnimationFsmSignal{anim_state_type});
			move_vector = direction_vector;
			move_force = 2800000.0f;
		}
		if (move_vector) {
			// Apply force
			phy.body->ApplyForceToCenter(static_cast<b2Vec2>(move_vector * (move_force * GAME.delta_time_s())), true);
		}

		// Primary weapon
		if (GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY)) {
			auto shoot = [&](const m2::Item& weapon) {
				auto [proj_obj, s] = m2::create_object(obj.position, id);
				rpg::create_ranged_weapon_object(proj_obj, vector_to_mouse, weapon);
				// Knock-back
				phy.body->ApplyForceToCenter(static_cast<b2Vec2>(m2::Vec2f::from_angle(vector_to_mouse.angle_rads() + m2::PI) * 50000.0f), true);
			};

			// Check if there is a special ranged weapon
			auto special_it = chr.find_items(m2g::pb::ITEM_CATEGORY_SPECIAL_RANGED_WEAPON);
			if (special_it != chr.end_items()) {
				// Check if special ammo left
				if (chr.has_resource(m2g::pb::RESOURCE_SPECIAL_RANGED_WEAPON_AMMO)) {
					// Try to use the weapon
					if (chr.use_item(special_it)) {
						shoot(*special_it);
					}
				} else {
					// Remove weapon if no ammo left
					chr.remove_item(special_it);
				}
			} else {
				// Find default weapon
				auto default_it = chr.find_items(m2g::pb::ITEM_CATEGORY_DEFAULT_RANGED_WEAPON);
				// Try to use the weapon
				if (chr.use_item(default_it)) {
					shoot(*default_it);
				}
			}
		}
		if (GAME.events.is_mouse_button_down(m2::MouseButton::SECONDARY) && obj.character().use_item(obj.character().find_items(m2g::pb::ITEM_REUSABLE_SWORD))) {
			auto& melee = m2::create_object(obj.position, id).first;
			rpg::create_melee_object(melee, vector_to_mouse, *GAME.get_item(m2g::pb::ITEM_REUSABLE_SWORD), true);
		}
	};
	chr.update = [](MAYBE m2::Character& chr) {
		if (not chr.has_resource(m2g::pb::RESOURCE_HP)) {
			LOG_INFO("You died");
			if (m2::ui::execute_blocking(rpg::Context::get_instance().you_died_menu()) == m2::ui::Action::QUIT) {
				GAME.quit = true;
			}
		}
	};
	phy.on_collision = [&phy, &chr](MAYBE m2::Physique& me, m2::Physique& other, MAYBE const m2::box2d::Contact& contact) {
		if (other.parent().character_id() && 10.0f < m2::Vec2f{phy.body->GetLinearVelocity()}.length()) {
			auto& other_char = other.parent().character();
			m2::Character::execute_interaction(chr, m2g::pb::InteractionType::STUN, other_char, m2g::pb::InteractionType::GET_STUNNED_BY);
		}
	};
	gfx.pre_draw = [&](m2::Graphic& gfx) {
		impl.animation_fsm.time(GAME.delta_time_s());
		gfx.draw_effect_health_bar = chr.get_resource(m2g::pb::RESOURCE_HP);
	};

	LEVEL.player_id = LEVEL.objects.get_id(&obj);
	return {};
}
