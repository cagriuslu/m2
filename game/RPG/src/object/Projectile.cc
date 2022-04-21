#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Def.h"
#include <rpg/object/Projectile.h>
#include <m2g/SpriteBlueprint.h>
#include <m2/box2d/Utils.h>
#include <m2/M2.h>

M2Err obj::Projectile::init(m2::Object& obj, const chr::ProjectileBlueprint* blueprint, ID originatorId, m2::Vec2f pos, m2::Vec2f dir) {
	obj = m2::Object{pos};
	dir = dir.normalize();

	auto& monitor = obj.add_monitor();

	auto& phy = obj.add_physique();
	phy.body = m2::box2d::create_bullet(
            *GAME.world,
			obj.physique_id,
			pos,
            true,
			m2::box2d::CATEGORY_PLAYER_BULLET,
			0.167f,
			0.0f,
			0.0f
	);
	phy.body->SetLinearVelocity(static_cast<b2Vec2>(dir * blueprint->speed_mps));

	auto& gfx = obj.add_graphic();
	gfx.textureRect = m2g::sprites[blueprint->sprite_index].texture_rect;
	gfx.center_px = m2g::sprites[blueprint->sprite_index].obj_center_px;
	gfx.angle = dir.angle_rads();

	auto& off = obj.add_offense();
    off.originator = originatorId;
	off.variant = chr::ProjectileState(blueprint);

	monitor.pre_phy = [&](m2::comp::Monitor& mon) {
		auto& projectile_state = std::get<chr::ProjectileState>(off.variant);
		m2::Vec2f curr_direction = m2::Vec2f{phy.body->GetLinearVelocity() }.normalize();
		phy.body->SetLinearVelocity(static_cast<b2Vec2>(curr_direction * projectile_state.blueprint->speed_mps));

		projectile_state.ttl_s -= GAME.deltaTicks_ms / 1000.0f;
		if (projectile_state.ttl_s <= 0) {
			Game_DeleteList_Add(mon.object_id);
		}
	};

	phy.on_collision = [&off](m2::comp::Physique& phy, m2::comp::Physique& other) {
		auto& other_obj = GAME.objects[other.object_id];
		auto& projectile_state = std::get<chr::ProjectileState>(off.variant);
		auto* def = GAME.defenses.get(other_obj.defense_id);
		if (def) {
			// Check if already collided
			if (projectile_state.already_collided_this_step) {
				return;
			}
			projectile_state.already_collided_this_step = true;
			// Calculate damage
			def->hp -= m2::apply_accuracy(projectile_state.blueprint->damage, projectile_state.blueprint->damage_accuracy);
			if (def->hp <= 0.0001f && def->on_death) {
				LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off.object_id, M2_ID, ID, def->object_id);
				def->on_death(*def);
			} else {
				LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off.object_id, M2_ID, ID, def->object_id, M2_HP, Float32, def->hp);
				auto direction = m2::Vec2f{phy.body->GetLinearVelocity() }.normalize();
				auto force = direction * 5000.0f;
				other.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
				if (def->on_hit) {
					def->on_hit(*def);
				}
			}
			Game_DeleteList_Add(phy.object_id);
		}
	};

	return 0;
}
