#include <b2_world.h>

#include <m2/object/Object.hh>
#include "m2/Box2DUtils.hh"
#include "m2/Game.hh"
#include "m2/Def.hh"
#include <game/ARPG_Cfg.hh>
#include <game/component.hh>

static b2Body* ObjectExplosive_CreateCollisionCircleBody(ID phyId, m2::vec2f position, const CfgExplosive *cfg) {
	return Box2DUtils_CreateBulletSensor(
		phyId,
		position,
		CATEGORY_PLAYER_BULLET,
		cfg->damageRadius_m,
		0.0f, // Mass
		0.0f // Damping
	);
}

static void ObjectExplosive_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId);
	ComponentPhysique* phy = Object_GetPhysique(obj);
	game::component_offense* off = Object_GetOffense(obj);

	switch (off->state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_IN_FLIGHT: {
			m2::vec2f curr_linear_velocity = m2::vec2f{ phy->body->GetLinearVelocity() };
			m2::vec2f new_linear_velocity = curr_linear_velocity.normalize() * off->state.explosive.cfg->projectileSpeed_mps;
			phy->body->SetLinearVelocity(static_cast<b2Vec2>(new_linear_velocity));
			off->state.explosive.projectileTtl_s -= GAME.deltaTicks_ms / 1000.0f;
			if (off->state.explosive.projectileTtl_s <= 0) {
				GAME.world->DestroyBody(phy->body);
				phy->body = ObjectExplosive_CreateCollisionCircleBody(obj->physique, obj->position, off->state.explosive.cfg);
				off->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP;
			}
			break;
		}
		case EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP:
            off->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP;
			break;
		default:
			break;
	}
}

static void ObjectExplosive_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	Object* obj = Game_FindObjectById(phy->super.objId);
	game::component_offense* off = Object_GetOffense(obj);

	switch (off->state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_IN_FLIGHT:
			// The object can collide with multiple targets during flight, thus this branch can be executed for multiple
			// other objects. This is not a problem since we only set the next state
            off->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP;
			break;
		case EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP: {
			Object* otherObj = GAME.objects.get(other->super.objId);
			if (otherObj->defense) {
				game::component_defense* defense = GAME.defenses.get(otherObj->defense);
				// Check if otherObj close enough. Colliding doesn't mean otherObj is in damage circle.
				float distance = otherObj->position.distance(obj->position);
				float damageRadius = off->state.explosive.cfg->damageRadius_m;
				if (distance < damageRadius) {
					// Calculate damage
					float minDamage = off->state.explosive.cfg->damageMin;
					float maxDamage = off->state.explosive.cfg->damageMax;
					float damage = LERP(maxDamage, minDamage, distance / damageRadius);
                    defense->hp -= damage;
					if (defense->hp <= 0.0001f && defense->onDeath) {
						// TODO fix XOK message
						LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off->super.objId, M2_ID, ID, defense->super.objId);
                        defense->onDeath(defense);
					} else {
						LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off->super.objId, M2_ID, ID, defense->super.objId, M2_HP, Float32, defense->hp);

						m2::vec2f curr_direction = m2::vec2f{ phy->body->GetLinearVelocity() }.normalize();
						m2::vec2f force = curr_direction * 5000.0f;
						other->body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
						if (defense->onHit) {
                            defense->onHit(defense);
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

static void ObjectExplosive_postPhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId);
	ComponentPhysique* phy = Object_GetPhysique(obj);
    game::component_offense* off = Object_GetOffense(obj);

	switch (off->state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP:
			Game_DeleteList_Add(el->super.objId);
			break;
		case EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP:
			GAME.world->DestroyBody(phy->body);
			phy->body = ObjectExplosive_CreateCollisionCircleBody(obj->physique, obj->position, off->state.explosive.cfg);
			break;
		default:
			break;
	}
}

M2Err ObjectExplosive_InitFromCfg(Object* obj, const CfgExplosive* cfg, ID originatorId, m2::vec2f position, m2::vec2f direction) {
	M2ERR_REFLECT(Object_Init(obj, position));
	direction = direction.normalize();

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = ObjectExplosive_prePhysics;
	el->postPhysics = ObjectExplosive_postPhysics;

	ComponentPhysique* phy = Object_AddPhysique(obj);
	phy->body = Box2DUtils_CreateBulletSensor(
        GAME.physics.get_id(phy),
		position,
		CATEGORY_PLAYER_BULLET,
		cfg->projectileBodyRadius_m,
		0.0f, // Mass
		0.0f // Damping
	);
	phy->body->SetLinearVelocity(static_cast<b2Vec2>(direction * cfg->projectileSpeed_mps));
	phy->onCollision = ObjectExplosive_onCollision;

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = ARPG_CFG_SPRITES[cfg->spriteIndex].textureRect;
	gfx->center_px = ARPG_CFG_SPRITES[cfg->spriteIndex].objCenter_px;
	gfx->angle = direction.angle_rads();

	game::component_offense* off = Object_AddOffense(obj);
    off->originator = originatorId;
    off->state.explosive.cfg = cfg;
    off->state.explosive.projectileTtl_s = cfg->projectileTtl_s;
    off->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_IN_FLIGHT;

	return M2OK;
}
