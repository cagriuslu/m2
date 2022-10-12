#include <rpg/object/Player.h>
#include <m2/Object.h>
#include <rpg/object/Enemy.h>
#include "m2/Game.hh"
#include "m2/Controls.h"
#include <rpg/object/Explosive.h>
#include <rpg/object/Projectile.h>
#include <rpg/object/Melee.h>
#include <m2g/component/Defense.h>
#include <m2/box2d/Utils.h>
#include <m2g/Object.h>
#include <m2/M2.h>
#include <Item.pb.h>
#include <m2/Log.h>

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
	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.set_allow_sleep(false);
	bp.set_is_bullet(false);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(GAME.sprites[blueprint->main_sprite].background_collider_circ_radius_m());
	bp.mutable_background_fixture()->set_is_sensor(false);
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND);
	// TODO foreground fixture
	bp.set_mass(blueprint->mass_kg);
	bp.set_linear_damping(blueprint->linear_damping);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), obj.position, bp);

	auto& gfx = obj.add_graphic(GAME.sprites[blueprint->main_sprite]);

	auto& light = obj.add_light();
	light.radius_m = 4.0f;

	auto& def = obj.add_defense();
    def.maxHp = def.hp = blueprint->max_hp;

	obj.impl = std::make_unique<obj::Player>(obj, blueprint);

	auto& monitor = obj.add_monitor();
	monitor.pre_phy = [&obj, &phy](MAYBE m2::comp::Monitor& mon) {
		auto* impl = dynamic_cast<obj::Player*>(obj.impl.get());
		auto to_mouse = (GAME.mousePositionWRTGameWorld_m - obj.position).normalize();

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
		if (GAME.events.pop_key_press(m2::Key::DASH) && impl->char_state.pop_dash()) {
			moveDirection = to_mouse;
			force = impl->char_state.blueprint->dash_force;
		} else {
			force = impl->char_state.blueprint->walk_force;
		}
		phy.body->ApplyForceToCenter(static_cast<b2Vec2>(moveDirection.normalize() * (GAME.deltaTicks_ms * force)), true);

		impl->char_state.process_time(GAME.deltaTime_s);

		if (GAME.events.is_mouse_button_down(m2::MouseButton::PRIMARY) && impl->char_state.ranged_weapon_state->blueprint->cooldown_s < impl->char_state.ranged_weapon_state->cooldown_counter_s) {
			auto& projectile = m2::create_object(obj.position).first;
			float accuracy = impl->char_state.blueprint->default_ranged_weapon->accuracy;
			float angle = to_mouse.angle_rads() + (m2::PI * m2::randf() * (1 - accuracy)) - (m2::PI * ((1 - accuracy) / 2.0f));
			obj::Projectile::init(projectile, &impl->char_state.blueprint->default_ranged_weapon->projectile, GAME.playerId, m2::Vec2f::from_angle(angle));
			// Knock-back
			phy.body->ApplyForceToCenter(static_cast<b2Vec2>(m2::Vec2f::from_angle(angle + m2::PI) * 500.0f), true);
			// TODO set looking direction here as well
			impl->char_state.ranged_weapon_state->cooldown_counter_s = 0;
		}
		if (GAME.events.is_mouse_button_down(m2::MouseButton::SECONDARY) && impl->char_state.melee_weapon_state->blueprint->cooldown_s < impl->char_state.melee_weapon_state->cooldown_counter_s) {
			auto& melee = m2::create_object(obj.position).first;
			obj::Melee::init(melee, &impl->char_state.blueprint->default_melee_weapon->melee, GAME.playerId, to_mouse);
			impl->char_state.melee_weapon_state->cooldown_counter_s = 0;
		}
		if (GAME.events.is_mouse_button_down(m2::MouseButton::MIDDLE) && impl->char_state.explosive_weapon_state->blueprint->cooldown_s < impl->char_state.explosive_weapon_state->cooldown_counter_s) {
			auto& explosive = m2::create_object(obj.position).first;
			obj::Explosive::init(explosive, &impl->char_state.blueprint->default_explosive_weapon->explosive, GAME.playerId, to_mouse);
			impl->char_state.explosive_weapon_state->cooldown_counter_s = 0;
		}
	};

	monitor.post_phy = [&obj, &phy, &def](MAYBE m2::comp::Monitor& mon) {
		auto* impl = dynamic_cast<obj::Player*>(obj.impl.get());
		// We must call time before other signals
		impl->animation_fsm.time(GAME.deltaTime_s);
		if (m2::Vec2f(phy.body->GetLinearVelocity()).is_small(0.5f)) {
			impl->animation_fsm.signal(m2g::pb::ANIMATION_STATE_IDLE);
		}
		// Consume consumables
		for (auto it = impl->items.begin(); it != impl->items.end(); ) {
			auto item = GAME.items[*it];
			if (item.usage() == m2::pb::Usage::CONSUMABLE) {
				for (const auto& resource : item.resources()) {
					switch (resource.type()) {
						case m2g::pb::ResourceType::HP:
							def.hp += (float)resource.amount();
							if (def.maxHp < def.hp) { def.hp = def.maxHp; }
							break;
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

	phy.on_collision = [&phy](MAYBE m2::comp::Physique& me, m2::comp::Physique& other) {
		auto* enemy_impl = dynamic_cast<obj::Enemy*>(other.parent().impl.get());
		if (enemy_impl && 10.0f < m2::Vec2f{phy.body->GetLinearVelocity()}.length()) {
			enemy_impl->stun();
		}
	};

	def.on_death = [](MAYBE m2g::comp::Defense& def) {
		LOG_INFO("Player died");
	};

	GAME.playerId = GAME.objects.get_id(&obj);
	return {};
}
