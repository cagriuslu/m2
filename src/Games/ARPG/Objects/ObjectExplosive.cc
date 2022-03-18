#include <m2/Object.hh>
#include <m2/Box2DUtils.hh>
#include <m2/Game.hh>
#include <m2/Def.hh>
#include "../ARPG_Cfg.hh"
#include "../ARPG_Component.hh"

static Box2DBody* ObjectExplosive_CreateCollisionCircleBody(ID phyId, m2::vec2f position, const CfgExplosive *cfg) {
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
	ComponentOffense* off = Object_GetOffense(obj);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(off->data);

	switch (offData->state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_IN_FLIGHT:
			Box2DBodySetLinearSpeed(phy->body, offData->state.explosive.cfg->projectileSpeed_mps);
			offData->state.explosive.projectileTtl_s -= GAME->deltaTicks_ms / 1000.0f;
			if (offData->state.explosive.projectileTtl_s <= 0) {
				Box2DWorldDestroyBody(GAME->world, phy->body);
				phy->body = ObjectExplosive_CreateCollisionCircleBody(obj->physique, obj->position, offData->state.explosive.cfg);
				offData->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP;
			}
			break;
		case EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP:
			offData->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP;
			break;
		default:
			break;
	}
}

static void ObjectExplosive_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	Object* obj = Game_FindObjectById(phy->super.objId);
	ComponentOffense* off = Object_GetOffense(obj);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(off->data);

	switch (offData->state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_IN_FLIGHT:
			// The object can collide with multiple targets during flight, thus this branch can be executed for multiple
			// other objects. This is not a problem since we only set the next state
			offData->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP;
			break;
		case EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP: {
			Object* otherObj = static_cast<Object *>(Pool_GetById(&GAME->objects,
																  other->super.objId)); M2ASSERT(otherObj);
			if (otherObj->defense) {
				ComponentDefense* defense = static_cast<ComponentDefense *>(Pool_GetById(&GAME->defenses,
																						 otherObj->defense)); M2ASSERT(defense);
				ARPG_ComponentDefense *defenseData = AS_ARPG_COMPONENTDEFENSE(defense->data);
				// Check if otherObj close enough. Colliding doesn't mean otherObj is in damage circle.
				float distance = otherObj->position.distance(obj->position);
				float damageRadius = offData->state.explosive.cfg->damageRadius_m;
				if (distance < damageRadius) {
					// Calculate damage
					float minDamage = offData->state.explosive.cfg->damageMin;
					float maxDamage = offData->state.explosive.cfg->damageMax;
					float damage = LERP(maxDamage, minDamage, distance / damageRadius);
					defenseData->hp -= damage;
					if (defenseData->hp <= 0.0001f && defenseData->onDeath) {
						// TODO fix XOK message
						LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off->super.objId, M2_ID, ID, defense->super.objId);
						defenseData->onDeath(defense);
					} else {
						LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off->super.objId, M2_ID, ID, defense->super.objId, M2_HP, Float32, defenseData->hp);
						Box2DBodyApplyForceToCenter(other->body, Box2DBodyGetLinearVelocity(phy->body).normalize() * 5000.0f, true);
						if (defenseData->onHit) {
							defenseData->onHit(defense);
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
	Object* obj = Game_FindObjectById(el->super.objId); M2ASSERT(obj);
	ComponentPhysique* phy = Object_GetPhysique(obj); M2ASSERT(phy);
	ComponentOffense* off = Object_GetOffense(obj); M2ASSERT(off);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(off->data);

	switch (offData->state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP:
			Game_DeleteList_Add(el->super.objId);
			break;
		case EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP:
			Box2DWorldDestroyBody(GAME->world, phy->body);
			phy->body = ObjectExplosive_CreateCollisionCircleBody(obj->physique, obj->position, offData->state.explosive.cfg);
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
		Pool_GetId(&GAME->physics, phy),
		position,
		CATEGORY_PLAYER_BULLET,
		cfg->projectileBodyRadius_m,
		0.0f, // Mass
		0.0f // Damping
	);
	Box2DBodySetLinearVelocity(phy->body, direction * cfg->projectileSpeed_mps);
	phy->onCollision = ObjectExplosive_onCollision;

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = ARPG_CFG_SPRITES[cfg->spriteIndex].textureRect;
	gfx->center_px = ARPG_CFG_SPRITES[cfg->spriteIndex].objCenter_px;
	gfx->angle = direction.angle_rads();

	ComponentOffense* off = Object_AddOffense(obj);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(off->data);
	offData->originator = originatorId;
	offData->state.explosive.cfg = cfg;
	offData->state.explosive.projectileTtl_s = cfg->projectileTtl_s;
	offData->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_IN_FLIGHT;

	return M2OK;
}
