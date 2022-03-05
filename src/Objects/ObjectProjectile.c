#include "../Object.h"
#include "../Game.h"
#include "../Box2DUtils.h"
#include "../Def.h"

static void Bullet_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); M2ASSERT(obj);
	ComponentPhysique* phy = Object_GetPhysique(obj); M2ASSERT(phy);
	ComponentOffense* offense = Object_GetOffense(obj); M2ASSERT(offense);
	Box2DBodySetLinearSpeed(phy->body, offense->state.projectile.cfg->speed_mps);

	offense->state.projectile.ttl_s -= GAME->deltaTicks / 1000.0f;
	if (offense->state.projectile.ttl_s <= 0) {
		Game_DeleteList_Add(el->super.objId);
	}
}

static void Bullet_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	Object* obj = Pool_GetById(&GAME->objects, phy->super.objId); M2ASSERT(obj);
	Object* otherObj = Pool_GetById(&GAME->objects, other->super.objId); M2ASSERT(otherObj);
	ComponentOffense* off = Pool_GetById(&GAME->offenses, obj->offense); M2ASSERT(off);
	ComponentDefense* defense = Pool_GetById(&GAME->defenses, otherObj->defense); M2ASSERT(defense);
	// Check if already collided
	if (off->state.projectile.alreadyCollidedThisStep) {
		return;
	}
	off->state.projectile.alreadyCollidedThisStep = true;
	// Calculate damage
	defense->hp -= ACCURACY(off->state.projectile.cfg->damage, off->state.projectile.cfg->damageAccuracy);
	if (defense->hp <= 0.0001f && defense->onDeath) {
		LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off->super.objId, M2_ID, ID, defense->super.objId);
		defense->onDeath(defense);
	} else {
		LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off->super.objId, M2_ID, ID, defense->super.objId, M2_HP, Float32, defense->hp);
		Box2DBodyApplyForceToCenter(other->body, Vec2F_Mul(Vec2F_Normalize(Box2DBodyGetLinearVelocity(phy->body)), 5000.0f), true);
		if (defense->onHit) {
			defense->onHit(defense);
		}
	}
	Game_DeleteList_Add(phy->super.objId);
}

int ObjectProjectile_InitFromCfg(Object* obj, const CfgProjectile *cfg, ID originatorId, Vec2F position, Vec2F direction) {
	M2ERR_REFLECT(Object_Init(obj, position, false));
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
	gfx->textureRect = cfg->texture->textureRect;
	gfx->center_px = cfg->texture->objCenter_px;
	gfx->angle = Vec2F_AngleRads(direction);
	gfx->motionBlurEnabled = true;
	gfx->prevObjGfxOriginWRTScreenCenter_px = ComponentGraphic_GraphicsOriginWRTScreenCenter_px(position, cfg->texture->objCenter_px);

	ComponentOffense* off = Object_AddOffense(obj);
	off->originator = originatorId;
	off->state.projectile.cfg = cfg;
	off->state.projectile.ttl_s = ACCURACY(cfg->ttl_s, cfg->ttlAccuracy);
	off->state.projectile.alreadyCollidedThisStep = false;

	return 0;
}
