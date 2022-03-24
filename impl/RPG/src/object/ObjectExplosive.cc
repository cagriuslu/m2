#include <b2_world.h>

#include <m2/Object.h>
#include "m2/Box2DUtils.hh"
#include "m2/Game.hh"
#include "m2/Def.hh"
#include "impl/private/ARPG_Cfg.hh"
#include "impl/public/Component.hh"

static b2Body* ObjectExplosive_CreateCollisionCircleBody(ID phyId, m2::Vec2f position, const CfgExplosive *cfg) {
	return Box2DUtils_CreateBulletSensor(
		phyId,
		position,
		CATEGORY_PLAYER_BULLET,
		cfg->damageRadius_m,
		0.0f, // Mass
		0.0f // Damping
	);
}

static void ObjectExplosive_prePhysics(m2::component::Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
	auto& phy = obj.physique();
	auto& off = obj.offense();

	switch (off.state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_IN_FLIGHT: {
			m2::Vec2f curr_linear_velocity = m2::Vec2f{phy.body->GetLinearVelocity() };
			m2::Vec2f new_linear_velocity = curr_linear_velocity.normalize() * off.state.explosive.cfg->projectileSpeed_mps;
			phy.body->SetLinearVelocity(static_cast<b2Vec2>(new_linear_velocity));
			off.state.explosive.projectileTtl_s -= GAME.deltaTicks_ms / 1000.0f;
			if (off.state.explosive.projectileTtl_s <= 0) {
				GAME.world->DestroyBody(phy.body);
				phy.body = ObjectExplosive_CreateCollisionCircleBody(obj.physique_id, obj.position, off.state.explosive.cfg);
				off.state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP;
			}
			break;
		}
		case EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP:
            off.state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP;
			break;
		default:
			break;
	}
}

static void ObjectExplosive_onCollision(m2::component::Physique& phy, m2::component::Physique& other) {
	auto& obj = GAME.objects[phy.object_id];
	auto& off = obj.offense();

	switch (off.state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_IN_FLIGHT:
			// The object can collide with multiple targets during flight, thus this branch can be executed for multiple
			// other objects. This is not a problem since we only set the next state
            off.state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP;
			break;
		case EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP: {
			auto& other_obj = GAME.objects[other.object_id];
			if (other_obj.defense_id) {
				auto& def = GAME.defenses[other_obj.defense_id];
				// Check if otherObj close enough. Colliding doesn't mean otherObj is in damage circle.
				float distance = other_obj.position.distance(obj.position);
				float damageRadius = off.state.explosive.cfg->damageRadius_m;
				if (distance < damageRadius) {
					// Calculate damage
					float minDamage = off.state.explosive.cfg->damageMin;
					float maxDamage = off.state.explosive.cfg->damageMax;
					float damage = LERP(maxDamage, minDamage, distance / damageRadius);
					def.hp -= damage;
					if (def.hp <= 0.0001f && def.onDeath) {
						// TODO fix XOK message
						LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off.object_id, M2_ID, ID, def.object_id);
						def.onDeath(&def);
					} else {
						LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off.object_id, M2_ID, ID, def.object_id, M2_HP, Float32, def.hp);

						m2::Vec2f curr_direction = m2::Vec2f{phy.body->GetLinearVelocity() }.normalize();
						m2::Vec2f force = curr_direction * 5000.0f;
						other.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
						if (def.onHit) {
							def.onHit(&def);
						}
					}
				}
			}
			break;
		}
		default:
			break;
	}
}

static void ObjectExplosive_postPhysics(m2::component::Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
	auto& phy = obj.physique();
	auto& off = obj.offense();

	switch (off.state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP:
			Game_DeleteList_Add(mon.object_id);
			break;
		case EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP:
			GAME.world->DestroyBody(phy.body);
			phy.body = ObjectExplosive_CreateCollisionCircleBody(obj.physique_id, obj.position, off.state.explosive.cfg);
			break;
		default:
			break;
	}
}

M2Err ObjectExplosive_InitFromCfg(m2::Object* obj, const CfgExplosive* cfg, ID originatorId, m2::Vec2f position, m2::Vec2f direction) {
	*obj = m2::Object{position};
	direction = direction.normalize();

	auto& mon = obj->add_monitor();
	mon.prePhysics = ObjectExplosive_prePhysics;
	mon.postPhysics = ObjectExplosive_postPhysics;

	auto& phy = obj->add_physique();
	phy.body = Box2DUtils_CreateBulletSensor(
			obj->physique_id,
			position,
			CATEGORY_PLAYER_BULLET,
			cfg->projectileBodyRadius_m,
			0.0f, // Mass
			0.0f // Damping
	);
	phy.body->SetLinearVelocity(static_cast<b2Vec2>(direction * cfg->projectileSpeed_mps));
	phy.onCollision = ObjectExplosive_onCollision;

	auto& gfx = obj->add_graphic();
	gfx.textureRect = ARPG_CFG_SPRITES[cfg->spriteIndex].textureRect;
	gfx.center_px = ARPG_CFG_SPRITES[cfg->spriteIndex].objCenter_px;
	gfx.angle = direction.angle_rads();

	auto& off = obj->add_offense();
    off.originator = originatorId;
    off.state.explosive.cfg = cfg;
    off.state.explosive.projectileTtl_s = cfg->projectileTtl_s;
    off.state.explosive.explosiveStatus = EXPLOSIVE_STATUS_IN_FLIGHT;

	return M2OK;
}
