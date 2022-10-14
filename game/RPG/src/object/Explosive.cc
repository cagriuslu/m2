#include <rpg/object/Explosive.h>
#include "rpg/ExplosiveWeapon.h"
#include <m2/Object.h>
#include "m2/Game.hh"
#include <m2g/Object.h>
#include <m2/box2d/Utils.h>
#include <m2/M2.h>
#include <m2/Log.h>

static b2Body* ObjectExplosive_CreateCollisionCircleBody(m2::Id phyId, m2::Vec2f position, const chr::ExplosiveBlueprint* blueprint) {
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.set_allow_sleep(false);
	bp.set_is_bullet(true);
	bp.mutable_background_fixture()->mutable_circ()->set_radius(blueprint->damage_radius_m);
	bp.mutable_background_fixture()->set_is_sensor(true);
	bp.mutable_background_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_BACKGROUND);
	bp.set_mass(0);
	bp.set_linear_damping(0);
	bp.set_fixed_rotation(true);
	return m2::box2d::create_body(*GAME.world, phyId, position, bp);
}

m2::VoidValue obj::Explosive::init(m2::Object& obj, const chr::ExplosiveBlueprint* blueprint, m2::ObjectId originator_id, m2::Vec2f direction) {
	direction = direction.normalize();

	auto& monitor = obj.add_monitor();

	auto& phy = obj.add_physique();
	m2::pb::BodyBlueprint bp;
	bp.set_type(m2::pb::BodyType::DYNAMIC);
	bp.set_allow_sleep(false);
	bp.set_is_bullet(true);
	bp.mutable_foreground_fixture()->mutable_circ()->set_radius(blueprint->projectile_body_radius_m);
	bp.mutable_foreground_fixture()->set_is_sensor(true);
	bp.mutable_foreground_fixture()->set_category(m2::pb::FixtureCategory::FRIEND_OFFENSE_ON_FOREGROUND);
	bp.set_mass(0);
	bp.set_linear_damping(0);
	bp.set_fixed_rotation(true);
	phy.body = m2::box2d::create_body(*GAME.world, obj.physique_id(), obj.position, bp);
	phy.body->SetLinearVelocity(static_cast<b2Vec2>(direction * blueprint->projectile_speed_mps));

	auto& gfx = obj.add_graphic(GAME.sprites[blueprint->sprite]);
	gfx.draw_angle = direction.angle_rads();

	auto& off = obj.add_offense();
    off.originator = originator_id;
	off.variant = chr::ExplosiveState(blueprint);

	monitor.pre_phy = [&](MAYBE m2::comp::Monitor& mon) {
		auto& explosive_state = std::get<chr::ExplosiveState>(off.variant);
		switch (explosive_state.status) {
			case chr::EXPLOSIVE_STATUS_IN_FLIGHT: {
				m2::Vec2f curr_linear_velocity = m2::Vec2f{phy.body->GetLinearVelocity() };
				m2::Vec2f new_linear_velocity = curr_linear_velocity.normalize() *  explosive_state.blueprint->projectile_speed_mps;
				phy.body->SetLinearVelocity(static_cast<b2Vec2>(new_linear_velocity));
				explosive_state.projectile_ttl_s -= GAME.deltaTicks_ms / 1000.0f;
				if (explosive_state.projectile_ttl_s <= 0) {
					m2::box2d::destroy_body(phy.body);
					phy.body = ObjectExplosive_CreateCollisionCircleBody(obj.physique_id(), obj.position, explosive_state.blueprint);
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
				m2::box2d::destroy_body(phy.body);
				phy.body = ObjectExplosive_CreateCollisionCircleBody(obj.physique_id(), obj.position, explosive_state.blueprint);
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
				if (other_obj.defense_id()) {
					auto& def = GAME.defenses[other_obj.defense_id()];
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
							LOG_TRACE("Projectile death", off.object_id, def.object_id);
							def.on_death(def);
						} else {
							LOG_TRACE("Projectile damage", off.object_id, def.object_id, def.hp);
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

	return {};
}
