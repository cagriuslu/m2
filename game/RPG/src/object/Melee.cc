#include <m2/Object.h>
#include "m2/Def.h"
#include "m2/Game.hh"
#include <rpg/object/Melee.h>
#include <m2g/SpriteBlueprint.h>
#include <m2/box2d/Utils.h>
#include <m2/Log.h>

#define SWING_SPEED (15.0f)

m2::VoidValue obj::Melee::init(m2::Object& obj, const chr::MeleeBlueprint *blueprint, m2::ID originatorId, m2::Vec2f position, m2::Vec2f direction) {
	obj = m2::Object{position};

	const float theta = direction.angle_rads(); // Convert direction to angle
	const float startAngle = theta + SWING_SPEED * (150 / 1000.0f / 2.0f);

	auto& monitor = obj.add_monitor();

	auto& phy = obj.add_physique();
	phy.body = m2::box2d::create_body(
            *GAME.world,
			obj.physique_id(),
            false,
            true,
            position,
            false,
            true,
            true,
            originatorId == GAME.playerId ? m2::box2d::CAT_PLAYER_AIR_OBJ : m2::box2d::CAT_ENEMY_AIR_OBJ,
            0,
            m2::Vec2f{1.25f, 0.1667f},
            m2::Vec2f{0.5833f, 0.0f},
            0.0f,
            NAN,
            1.0f,
            0.0f,
            false
	);
	phy.body->SetTransform(static_cast<b2Vec2>(position), startAngle);
	phy.body->SetAngularVelocity(-SWING_SPEED);

	auto& gfx = obj.add_graphic();
	gfx.textureRect = m2g::sprites[blueprint->sprite_index].texture_rect;
	gfx.center_px = m2g::sprites[blueprint->sprite_index].obj_center_px;
	gfx.angle = phy.body->GetAngle();

	auto& off = obj.add_offense();
    off.originator = originatorId;
	off.variant = chr::MeleeState(blueprint);

	monitor.pre_phy = [&](m2::comp::Monitor& mon) {
		auto& melee_state = std::get<chr::MeleeState>(off.variant);
		melee_state.ttl_s -= GAME.deltaTicks_ms / 1000.0f;
		if (melee_state.ttl_s <= 0) {
			GAME.add_deferred_action(m2::create_object_deleter(mon.object_id));
		}
	};

	monitor.post_phy = [&](MAYBE m2::comp::Monitor& mon) {
		float angle = phy.body->GetAngle();
		m2::Object* originator = GAME.objects.get(off.originator);
		if (originator) {
			// Make sure originator is still alive
			phy.body->SetTransform(static_cast<b2Vec2>(originator->position), angle);
		}
		gfx.angle = angle;
	};

	phy.on_collision = [&](MAYBE m2::comp::Physique& phy, m2::comp::Physique& other) {
		LOG_DEBUG("Collision");
		auto& other_obj = GAME.objects[other.object_id];
		auto& melee_state = std::get<chr::MeleeState>(off.variant);
		auto& def = GAME.defenses[other_obj.defense_id()];

		// Calculate damage
		def.hp -= melee_state.blueprint->damage;
		if (def.hp <= 0.0001f && def.on_death) {
			LOG_TRACE("Projectile death", off.object_id, def.object_id);
			def.on_death(def);
		} else {
			LOG_TRACE("Projectile damage", off.object_id, def.object_id, def.hp);
			auto direction = (other_obj.position - obj.position).normalize();
			auto force = direction * 15000.0f;
			other.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
			if (def.on_hit) {
				def.on_hit(def);
			}
		}
	};
	
	return {};
}
