#include <rpg/object/Player.h>
#include <m2/Object.h>
#include <rpg/object/Enemy.h>
#include "m2/Game.hh"
#include "m2/Controls.h"
#include "m2/Def.h"
#include <rpg/object/Explosive.h>
#include <rpg/object/Projectile.h>
#include <rpg/object/Melee.h>
#include <m2g/component/Defense.h>
#include <m2/box2d/Utils.h>
#include <m2g/SpriteBlueprint.h>
#include <m2/M2.h>
#include <m2/Log.h>

obj::Player::Player(m2::Object& obj, const chr::CharacterBlueprint* blueprint) : char_state(blueprint), char_animator({obj.graphic(), blueprint}) {}

void obj::Player::add_consumable(const itm::ConsumableBlueprint& consumable) {
	consumables.emplace_back(consumable);
}

// Mouse primary button: shoot projectile (player can at most carry 3 primary weapons)
// Mouse secondary button: melee weapon (player can only carry one melee weapon)
// Mouse middle button: explosive weapon (player can only carry one explosive weapon)
// Mouse middle scroll: change primary projectile weapon
// Double tap directional buttons to dodge

m2::VoidValue obj::Player::init(m2::Object& obj, const chr::CharacterBlueprint* blueprint, m2::Vec2f pos) {
	obj = m2::Object{pos};

	auto& monitor = obj.add_monitor();

	auto& phy = obj.add_physique();
	phy.body = m2::box2d::create_dynamic_disk(
            *GAME.world,
            obj.physique_id(),
            obj.position,
            false,
            false,
            m2::box2d::CAT_PLAYER,
            std::get<m2::ColliderBlueprint::Circle>(m2g::sprites[blueprint->main_sprite_index].collider.variant).radius_m,
            blueprint->mass_kg,
            blueprint->linear_damping
	);

	auto& gfx = obj.add_graphic();
	gfx.textureRect = m2g::sprites[blueprint->main_sprite_index].texture_rect;
	gfx.center_px = m2g::sprites[blueprint->main_sprite_index].obj_center_px;

	auto& light = obj.add_light();
	light.radius_m = 4.0f;

	auto& def = obj.add_defense();
    def.maxHp = def.hp = blueprint->max_hp;

	obj.impl = std::make_unique<obj::Player>(obj, blueprint);

	monitor.pre_phy = [&obj, &phy]([[maybe_unused]] m2::comp::Monitor& mon) {
		auto* impl = dynamic_cast<obj::Player*>(obj.impl.get());
		auto to_mouse = (GAME.mousePositionInWorld_m - obj.position).normalize();

		m2::Vec2f moveDirection;
		if (GAME.events.is_key_down(m2::Key::UP)) {
			moveDirection.y += -1.0f;
			impl->char_animator.signal(fsm::CharacterAnimation::CHARANIM_WALKUP);
		}
		if (GAME.events.is_key_down(m2::Key::DOWN)) {
			moveDirection.y += 1.0f;
			impl->char_animator.signal(fsm::CharacterAnimation::CHARANIM_WALKDOWN);
		}
		if (GAME.events.is_key_down(m2::Key::LEFT)) {
			moveDirection.x += -1.0f;
			impl->char_animator.signal(fsm::CharacterAnimation::CHARANIM_WALKLEFT);
		}
		if (GAME.events.is_key_down(m2::Key::RIGHT)) {
			moveDirection.x += 1.0f;
			impl->char_animator.signal(fsm::CharacterAnimation::CHARANIM_WALKRIGHT);
		}
		float force;
		if (GAME.events.pop_key_press(m2::Key::DASH) && impl->char_state.pop_dash()) {
			moveDirection = to_mouse;
			force = impl->char_state.blueprint->dash_force;
		} else {
			force = impl->char_state.blueprint->walk_force;
		}
		phy.body->ApplyForceToCenter(static_cast<b2Vec2>(moveDirection.normalize() * (GAME.deltaTicks_ms * force)), true);

		impl->char_state.process_time(GAME.deltaTime_s);

		if (GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY) && impl->char_state.ranged_weapon_state->blueprint->cooldown_s < impl->char_state.ranged_weapon_state->cooldown_counter_s) {
			auto& projectile = GAME.objects.alloc().first;
			float accuracy = impl->char_state.blueprint->default_ranged_weapon->accuracy;
			float angle = to_mouse.angle_rads() + (M2_PI * m2::randf() * (1 - accuracy)) - (M2_PI * ((1 - accuracy) / 2.0f));
			obj::Projectile::init(projectile, &impl->char_state.blueprint->default_ranged_weapon->projectile, GAME.playerId, obj.position, m2::Vec2f::from_angle(angle));
			// Knockback
			phy.body->ApplyForceToCenter(static_cast<b2Vec2>(m2::Vec2f::from_angle(angle + M2_PI) * 500.0f), true);
			// TODO set looking direction here as well
			impl->char_state.ranged_weapon_state->cooldown_counter_s = 0;
		}
		if (GAME.events.is_mouse_button_down(m2::MouseButton::SECONDARY) && impl->char_state.melee_weapon_state->blueprint->cooldown_s < impl->char_state.melee_weapon_state->cooldown_counter_s) {
			auto& melee = GAME.objects.alloc().first;
			obj::Melee::init(melee, &impl->char_state.blueprint->default_melee_weapon->melee, GAME.playerId, obj.position, to_mouse);
			impl->char_state.melee_weapon_state->cooldown_counter_s = 0;
		}
		if (GAME.events.is_mouse_button_down(m2::MouseButton::MIDDLE) && impl->char_state.explosive_weapon_state->blueprint->cooldown_s < impl->char_state.explosive_weapon_state->cooldown_counter_s) {
			auto& explosive = GAME.objects.alloc().first;
			obj::Explosive::init(explosive, &impl->char_state.blueprint->default_explosive_weapon->explosive, GAME.playerId, obj.position, to_mouse);
			impl->char_state.explosive_weapon_state->cooldown_counter_s = 0;
		}
	};

	monitor.post_phy = [&obj, &phy, &def]([[maybe_unused]] m2::comp::Monitor& mon) {
		auto* impl = dynamic_cast<obj::Player*>(obj.impl.get());
		// We must call time before other signals
		impl->char_animator.time(GAME.deltaTime_s);
		if (m2::Vec2f(phy.body->GetLinearVelocity()).is_small(0.5f)) {
			impl->char_animator.signal(fsm::CharacterAnimation::CHARANIM_STOP);
		}
		// Consumables
		for (auto& consumable : impl->consumables) {
			for (const auto& buff : consumable.buffs) {
				switch (buff.first) {
					case chr::Attribute::HP:
						def.hp += buff.second;
						if (def.maxHp < def.hp) { def.hp = def.maxHp; }
						break;
					default:
						break;
				}
			}
		}
		impl->consumables.clear();
	};

	phy.on_collision = [&phy](m2::comp::Physique& me, m2::comp::Physique& other) {
		auto* enemy_impl = dynamic_cast<obj::Enemy*>(other.parent().impl.get());
		if (enemy_impl && 10.0f < m2::Vec2f{phy.body->GetLinearVelocity()}.length()) {
			enemy_impl->stun();
		}
	};

	def.on_death = []([[maybe_unused]] m2g::comp::Defense& def) {
		LOG_INFO("Player died");
	};

	GAME.playerId = GAME.objects.get_id(&obj);
	return {};
}
