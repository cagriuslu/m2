#include <m2/Object.h>
#include <m2/Game.h>
#include <m2/Box2DUtils.h>
#include <m2/Def.h>
#include "../ARPG_Component.h"
#include "../ARPG_Cfg.h"

static void Bullet_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); M2ASSERT(obj);
	ComponentPhysique* phy = Object_GetPhysique(obj); M2ASSERT(phy);
	ComponentOffense* offense = Object_GetOffense(obj); M2ASSERT(offense);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(offense->data);
	Box2DBodySetLinearSpeed(phy->body, offData->state.projectile.cfg->speed_mps);

	offData->state.projectile.ttl_s -= GAME->deltaTicks_ms / 1000.0f;
	if (offData->state.projectile.ttl_s <= 0) {
		Game_DeleteList_Add(el->super.objId);
	}
}

static void Bullet_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	Object* obj = Pool_GetById(&GAME->objects, phy->super.objId); M2ASSERT(obj);
	Object* otherObj = Pool_GetById(&GAME->objects, other->super.objId); M2ASSERT(otherObj);
	ComponentOffense* off = Pool_GetById(&GAME->offenses, obj->offense); M2ASSERT(off);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(off->data);
	ComponentDefense* defense = Pool_GetById(&GAME->defenses, otherObj->defense); M2ASSERT(defense);
	ARPG_ComponentDefense *defenseData = AS_ARPG_COMPONENTDEFENSE(defense->data);
	// Check if already collided
	if (offData->state.projectile.alreadyCollidedThisStep) {
		return;
	}
	offData->state.projectile.alreadyCollidedThisStep = true;
	// Calculate damage
	defenseData->hp -= ACCURACY(offData->state.projectile.cfg->damage, offData->state.projectile.cfg->damageAccuracy);
	if (defenseData->hp <= 0.0001f && defenseData->onDeath) {
		LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off->super.objId, M2_ID, ID, defense->super.objId);
		defenseData->onDeath(defense);
	} else {
		LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off->super.objId, M2_ID, ID, defense->super.objId, M2_HP, Float32, defenseData->hp);
		Box2DBodyApplyForceToCenter(other->body, Vec2F_Mul(Vec2F_Normalize(Box2DBodyGetLinearVelocity(phy->body)), 5000.0f), true);
		if (defenseData->onHit) {
			defenseData->onHit(defense);
		}
	}
	Game_DeleteList_Add(phy->super.objId);
}

int ObjectProjectile_InitFromCfg(Object* obj, const CfgProjectile *cfg, ID originatorId, Vec2F position, Vec2F direction) {
	M2ERR_REFLECT(Object_Init(obj, position));
	direction = Vec2F_Normalize(direction);

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = Bullet_prePhysics;

	ComponentPhysique* phy = Object_AddPhysique(obj);
	phy->body = Box2DUtils_CreateBulletSensor(
		Pool_GetId(&GAME->physics, phy),
		position,
		CATEGORY_PLAYER_BULLET,
		0.167f, // Radius
		0.0f, // Mass
		0.0f // Damping
	);
	Box2DBodySetLinearVelocity(phy->body, Vec2F_Mul(direction, cfg->speed_mps));
	phy->onCollision = Bullet_onCollision;
	
	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = ARPG_CFG_SPRITES[cfg->spriteIndex].textureRect;
	gfx->center_px = ARPG_CFG_SPRITES[cfg->spriteIndex].objCenter_px;
	gfx->angle = Vec2F_AngleRads(direction);

	ComponentOffense* off = Object_AddOffense(obj);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(off->data);
	offData->originator = originatorId;
	offData->state.projectile.cfg = cfg;
	offData->state.projectile.ttl_s = ACCURACY(cfg->ttl_s, cfg->ttlAccuracy);
	offData->state.projectile.alreadyCollidedThisStep = false;

	return 0;
}
