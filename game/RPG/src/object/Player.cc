#include <rpg/object/Player.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Controls.h"
#include "m2/Event.hh"
#include "m2/Def.h"
#include <rpg/object/Explosive.h>
#include <rpg/object/Projectile.h>
#include <rpg/object/Melee.h>
#include <m2g/component/Defense.h>
#include <m2g/component/Offense.h>
#include <m2/box2d/Utils.h>
#include <m2g/SpriteBlueprint.h>
#include <m2/M2.h>

impl::object::Player::Player(m2::Object& obj, const character::CharacterBlueprint* blueprint) : char_state(blueprint), char_animator({obj.graphic(), blueprint}) {}

// Mouse primary button: shoot projectile (player can at most carry 3 primary weapons)
// Mouse secondary button: melee weapon (player can only carry one melee weapon)
// Mouse middle button: explosive weapon (player can only carry one explosive weapon)
// Mouse middle scroll: change primary projectile weapon
// Double tap directional buttons to dodge

M2Err impl::object::Player::init(m2::Object& obj, const character::CharacterBlueprint* blueprint, m2::Vec2f pos) {
	obj = m2::Object{pos};

	auto& monitor = obj.add_monitor();

	auto& phy = obj.add_physique();
	phy.body = m2::box2d::create_dynamic_disk(
            *GAME.world,
            obj.physique_id,
            obj.position,
            false,
            false,
            m2::box2d::CATEGORY_PLAYER,
            std::get<m2::ColliderBlueprint::Circle>(impl::sprites[blueprint->main_sprite_index].collider.variant).radius_m,
            blueprint->mass_kg,
            blueprint->linear_damping
	);

	auto& gfx = obj.add_graphic();
	gfx.textureRect = impl::sprites[blueprint->main_sprite_index].texture_rect;
	gfx.center_px = impl::sprites[blueprint->main_sprite_index].obj_center_px;

	auto& light = obj.add_light();
	light.radius_m = 4.0f;

	auto& def = obj.add_defense();
    def.maxHp = def.hp = blueprint->max_hp;

	obj.impl = std::make_unique<impl::object::Player>(obj, blueprint);

	monitor.pre_phy = [&]([[maybe_unused]] m2::component::Monitor& mon) {
		auto* data = dynamic_cast<impl::object::Player*>(obj.impl.get());
		m2::Vec2f moveDirection;
		if (GAME.events.key_down[m2::u(m2::Key::UP)]) {
			moveDirection.y += -1.0f;
			data->char_animator.signal(impl::fsm::CharacterAnimation::CHARANIM_WALKUP);
		}
		if (GAME.events.key_down[m2::u(m2::Key::DOWN)]) {
			moveDirection.y += 1.0f;
			data->char_animator.signal(impl::fsm::CharacterAnimation::CHARANIM_WALKDOWN);
		}
		if (GAME.events.key_down[m2::u(m2::Key::LEFT)]) {
			moveDirection.x += -1.0f;
			data->char_animator.signal(impl::fsm::CharacterAnimation::CHARANIM_WALKLEFT);
		}
		if (GAME.events.key_down[m2::u(m2::Key::RIGHT)]) {
			moveDirection.x += 1.0f;
			data->char_animator.signal(impl::fsm::CharacterAnimation::CHARANIM_WALKRIGHT);
		}

		auto& phy = GAME.physics[obj.physique_id];
		phy.body->ApplyForceToCenter(static_cast<b2Vec2>(moveDirection.normalize() * (GAME.deltaTicks_ms * 25.0f)), true);

		data->char_state.explosive_weapon_state->process_time(GAME.deltaTime_s);
		data->char_state.melee_weapon_state->process_time(GAME.deltaTime_s);
		data->char_state.ranged_weapon_state->process_time(GAME.deltaTime_s);

		if (GAME.events.mouse_button_down[m2::u(m2::MouseButton::PRIMARY)] && data->char_state.ranged_weapon_state->blueprint->cooldown_s < data->char_state.ranged_weapon_state->cooldown_counter_s) {
			auto& projectile = GAME.objects.alloc().first;
			m2::Vec2f direction = (GAME.mousePositionInWorld_m - obj.position).normalize();
			float accuracy = data->char_state.blueprint->default_ranged_weapon->accuracy;
			float angle = direction.angle_rads() + (M2_PI * m2::randf() * (1 - accuracy)) - (M2_PI * ((1 - accuracy) / 2.0f));
			impl::object::Projectile::init(projectile, &data->char_state.blueprint->default_ranged_weapon->projectile, GAME.playerId, obj.position, m2::Vec2f::from_angle(angle));
			// Knockback
			phy.body->ApplyForceToCenter(static_cast<b2Vec2>(m2::Vec2f::from_angle(angle + M2_PI) * 500.0f), true);
			// TODO set looking direction here as well
			data->char_state.ranged_weapon_state->cooldown_counter_s = 0;
		}
		if (GAME.events.mouse_button_down[m2::u(m2::MouseButton::SECONDARY)] && data->char_state.melee_weapon_state->blueprint->cooldown_s < data->char_state.melee_weapon_state->cooldown_counter_s) {
			auto& melee = GAME.objects.alloc().first;
			impl::object::Melee::init(melee, &data->char_state.blueprint->default_melee_weapon->melee, GAME.playerId, obj.position, GAME.mousePositionInWorld_m - obj.position);
			data->char_state.ranged_weapon_state->cooldown_counter_s = 0;
		}
		if (GAME.events.mouse_button_down[m2::u(m2::MouseButton::MIDDLE)] && data->char_state.explosive_weapon_state->blueprint->cooldown_s < data->char_state.explosive_weapon_state->cooldown_counter_s) {
			auto& explosive = GAME.objects.alloc().first;
			impl::object::Explosive::init(explosive, &data->char_state.blueprint->default_explosive_weapon->explosive, GAME.playerId, obj.position, GAME.mousePositionInWorld_m - obj.position);
			data->char_state.explosive_weapon_state->cooldown_counter_s = 0;
		}
	};

	monitor.post_phy = [&]([[maybe_unused]] m2::component::Monitor& mon) {
		auto* data = dynamic_cast<impl::object::Player*>(obj.impl.get());
		// We must call time before other signals
		data->char_animator.time(GAME.deltaTime_s);
		m2::Vec2f velocity = m2::Vec2f{phy.body->GetLinearVelocity() };
		if (fabsf(velocity.x) < 0.5000f && fabsf(velocity.y) < 0.5000f) {
			data->char_animator.signal(impl::fsm::CharacterAnimation::CHARANIM_STOP);
		}
	};

	def.on_death = []([[maybe_unused]] impl::component::Defense& def) {
		LOG_INFO("Player died");
	};

	GAME.playerId = GAME.objects.get_id(&obj);
	return M2OK;
}
