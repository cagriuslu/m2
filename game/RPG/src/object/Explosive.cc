#include <rpg/object/Explosive.h>
#include "rpg/ExplosiveWeapon.h"
#include <b2_world.h>
#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Def.h"
#include <m2g/SpriteBlueprint.h>
#include <m2/box2d/Utils.h>
#include <m2/M2.h>

static b2Body* ObjectExplosive_CreateCollisionCircleBody(ID phyId, m2::Vec2f position,  const chr::ExplosiveBlueprint* blueprint) {
	return m2::box2d::create_bullet(
            *GAME.world,
            phyId,
            position,
            true,
            m2::box2d::CAT_PLAYER_AOE,
			blueprint->damage_radius_m,
            0.0f,
            0.0f
	);
}

M2Err obj::Explosive::init(m2::Object& obj, const chr::ExplosiveBlueprint* blueprint, m2::ObjectID originator_id, m2::Vec2f position, m2::Vec2f direction) {
	obj = m2::Object{position};
	direction = direction.normalize();

	auto& monitor = obj.add_monitor();

	auto& phy = obj.add_physique();
	phy.body = m2::box2d::create_bullet(
            *GAME.world,
			obj.physique_id,
			position,
            true,
			m2::box2d::CAT_PLAYER_AIR_OBJ,
			blueprint->projectile_body_radius_m,
			0.0f,
			0.0f
	);
	phy.body->SetLinearVelocity(static_cast<b2Vec2>(direction * blueprint->projectile_speed_mps));

	auto& gfx = obj.add_graphic();
	gfx.textureRect = m2g::sprites[blueprint->sprite_index].texture_rect;
	gfx.center_px = m2g::sprites[blueprint->sprite_index].obj_center_px;
	gfx.angle = direction.angle_rads();

	auto& off = obj.add_offense();
    off.originator = originator_id;
	off.variant = chr::ExplosiveState(blueprint);

	monitor.pre_phy = [&]([[maybe_unused]] m2::comp::Monitor& mon) {
		auto& explosive_state = std::get<chr::ExplosiveState>(off.variant);
		switch (explosive_state.status) {
			case chr::EXPLOSIVE_STATUS_IN_FLIGHT: {
				m2::Vec2f curr_linear_velocity = m2::Vec2f{phy.body->GetLinearVelocity() };
				m2::Vec2f new_linear_velocity = curr_linear_velocity.normalize() *  explosive_state.blueprint->projectile_speed_mps;
				phy.body->SetLinearVelocity(static_cast<b2Vec2>(new_linear_velocity));
				explosive_state.projectile_ttl_s -= GAME.deltaTicks_ms / 1000.0f;
				if (explosive_state.projectile_ttl_s <= 0) {
					GAME.world->DestroyBody(phy.body);
					phy.body = ObjectExplosive_CreateCollisionCircleBody(obj.physique_id, obj.position, explosive_state.blueprint);
					explosive_state.status = chr::EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP;
				}
				break;
			}
			case chr::EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP:
				explosive_state.status = chr::EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP;
				break;
			default:
				break;
		}
	};

	monitor.post_phy = [&](m2::comp::Monitor& mon) {
		auto& explosive_state = std::get<chr::ExplosiveState>(off.variant);
		switch (explosive_state.status) {
			case chr::EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP:
				GAME.add_deferred_action(m2::create_object_deleter(mon.object_id));
				break;
			case chr::EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP:
				GAME.world->DestroyBody(phy.body);
				phy.body = ObjectExplosive_CreateCollisionCircleBody(obj.physique_id, obj.position, explosive_state.blueprint);
				break;
			default:
				break;
		}
	};

	phy.on_collision = [&](m2::comp::Physique& phy, m2::comp::Physique& other) {
		auto& explosive_state = std::get<chr::ExplosiveState>(off.variant);
		switch (explosive_state.status) {
			case chr::EXPLOSIVE_STATUS_IN_FLIGHT:
				// The object can collide with multiple targets during flight, thus this branch can be executed for multiple
				// other objects. This is not a problem since we only set the next state
				explosive_state.status = chr::EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP;
				break;
			case chr::EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP: {
				auto& other_obj = GAME.objects[other.object_id];
				if (other_obj.defense_id) {
					auto& def = GAME.defenses[other_obj.defense_id];
					// Check if otherObj close enough. Colliding doesn't mean otherObj is in damage circle.
					float distance = other_obj.position.distance(obj.position);
					float damageRadius = explosive_state.blueprint->damage_radius_m;
					if (distance < damageRadius) {
						// Calculate damage
						float minDamage = explosive_state.blueprint->damage_min;
						float maxDamage = explosive_state.blueprint->damage_max;
						float damage = m2::lerp(maxDamage, minDamage, distance / damageRadius);
						def.hp -= damage;
						if (def.hp <= 0.0001f && def.on_death) {
							// TODO fix XOK message
							LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off.object_id, M2_ID, ID, def.object_id);
							def.on_death(def);
						} else {
							LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off.object_id, M2_ID, ID, def.object_id, M2_HP, Float32, def.hp);

							m2::Vec2f curr_direction = m2::Vec2f{phy.body->GetLinearVelocity() }.normalize();
							m2::Vec2f force = curr_direction * 5000.0f;
							other.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
							if (def.on_hit) {
								def.on_hit(def);
							}
						}
					}
				}
				break;
			}
			default:
				break;
		}
	};

	return M2OK;
}
