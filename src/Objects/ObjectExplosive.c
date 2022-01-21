#include "../Object.h"
#include "../Box2DUtils.h"
#include "../Game.h"
#include "../Def.h"

static Box2DBody* ObjectExplosive_CreateCollisionCircleBody(ID phyId, Vec2F position, const CfgExplosive *cfg) {
	return Box2DUtils_CreateBulletSensor(
			phyId,
			position,
			CATEGORY_PLAYER_BULLET,
			cfg->damageRadius,
			0.0f, // Mass
			0.0f // Damping
	);
}

static void ObjectExplosive_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); XASSERT(obj);
	ComponentPhysique* phy = Object_GetPhysique(obj); XASSERT(phy && phy->body);
	ComponentOffense* off = Object_GetOffense(obj); XASSERT(off);

	switch (off->state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_IN_FLIGHT:
			Box2DBodySetLinearSpeed(phy->body, off->state.explosive.cfg->projectileSpeed);
			off->state.explosive.projectileTtl -= GAME->deltaTicks / 1000.0f;
			if (off->state.explosive.projectileTtl <= 0) {
				Box2DWorldDestroyBody(GAME->world, phy->body);
				phy->body = ObjectExplosive_CreateCollisionCircleBody(obj->physique, obj->position, off->state.explosive.cfg);
				off->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP;
			}
			break;
		case EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP:
			off->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP;
			break;
		default:
			break;
	}
}

static void ObjectExplosive_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	Object* obj = Game_FindObjectById(phy->super.objId); XASSERT(obj);
	ComponentOffense* off = Object_GetOffense(obj); XASSERT(off);

	switch (off->state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_IN_FLIGHT:
			// The object can collide with multiple targets during flight, thus this branch can be executed for multiple
			// other objects. This is not a problem since we only set the next state
			off->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP;
			break;
		case EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP: {
			Object* otherObj = Pool_GetById(&GAME->objects, other->super.objId); XASSERT(otherObj);
			if (otherObj->defense) {
				ComponentDefense* defense = Pool_GetById(&GAME->defenses, otherObj->defense); XASSERT(defense);
				// Check if otherObj close enough. Colliding doesn't mean otherObj is in damage circle.
				float distance = Vec2F_Distance(otherObj->position, obj->position);
				float damageRadius = off->state.explosive.cfg->damageRadius;
				if (distance < damageRadius) {
					// Calculate damage
					float minDamage = off->state.explosive.cfg->damageMin;
					float maxDamage = off->state.explosive.cfg->damageMax;
					float damage = LERP(maxDamage, minDamage, distance / damageRadius);
					defense->hp -= damage;
					if (defense->hp <= 0.0001f && defense->onDeath) {
						// TODO fix XOK message
						LOG2XV_TRC(XOK_PROJECTILE_DEATH, ID, off->super.objId, XOK_ID, ID, defense->super.objId);
						defense->onDeath(defense);
					} else {
						LOG3XV_TRC(XOK_PROJECTILE_DMG, ID, off->super.objId, XOK_ID, ID, defense->super.objId, XOK_HP, Float32, defense->hp);
						Box2DBodyApplyForceToCenter(other->body, Vec2F_Mul(Vec2F_Normalize(Box2DBodyGetLinearVelocity(phy->body)), 5000.0f), true);
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
	Object* obj = Game_FindObjectById(el->super.objId); XASSERT(obj);
	ComponentPhysique* phy = Object_GetPhysique(obj); XASSERT(phy);
	ComponentOffense* off = Object_GetOffense(obj); XASSERT(off);

	switch (off->state.explosive.explosiveStatus) {
		case EXPLOSIVE_STATUS_WILL_EXPLODE_THIS_STEP:
			Game_DeleteList_Add(el->super.objId);
			break;
		case EXPLOSIVE_STATUS_WILL_EXPLODE_NEXT_STEP:
			Box2DWorldDestroyBody(GAME->world, phy->body);
			phy->body = ObjectExplosive_CreateCollisionCircleBody(obj->physique, obj->position, off->state.explosive.cfg);
			break;
		default:
			break;
	}
}

XErr ObjectExplosive_InitFromCfg(Object* obj, const CfgExplosive* cfg, ID originatorId, Vec2F position, Vec2F direction) {
	direction = Vec2F_Normalize(direction);
	XERR_REFLECT(Object_Init(obj, position, false));

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = ObjectExplosive_prePhysics;
	el->postPhysics = ObjectExplosive_postPhysics;

	ComponentPhysique* phy = Object_AddPhysique(obj);
	phy->body = Box2DUtils_CreateBulletSensor(
		Pool_GetId(&GAME->physics, phy),
		position,
		CATEGORY_PLAYER_BULLET,
		cfg->projectileRadius,
		0.0f, // Mass
		0.0f // Damping
	);
	Box2DBodySetLinearVelocity(phy->body, Vec2F_Mul(direction, cfg->projectileSpeed));
	phy->onCollision = ObjectExplosive_onCollision;

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = cfg->texture->textureRect;
	gfx->center_px = cfg->texture->objCenter_px;
	gfx->angle = Vec2F_AngleRads(direction);

	ComponentOffense* off = Object_AddOffense(obj);
	off->originator = originatorId;
	off->state.explosive.cfg = cfg;
	off->state.explosive.projectileTtl = cfg->projectileTtl;
	off->state.explosive.explosiveStatus = EXPLOSIVE_STATUS_IN_FLIGHT;

	return XOK;
}
