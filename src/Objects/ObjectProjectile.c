#include "../Object.h"
#include "../Game.h"
#include "../Box2DUtils.h"
#include "../Def.h"

static void Bullet_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); XASSERT(obj);
	ComponentPhysique* phy = Object_GetPhysique(obj); XASSERT(phy);
	ComponentOffense* offense = Object_GetOffense(obj); XASSERT(offense);
	Box2DBodySetLinearSpeed(phy->body, offense->state.projectile.cfg->speed);

	offense->state.projectile.ttl -= GAME->deltaTicks / 1000.0f;
	if (offense->state.projectile.ttl <= 0) {
		Game_DeleteList_Add(el->super.objId);
	}
}

static void Bullet_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	Object* obj = Pool_GetById(&GAME->objects, phy->super.objId); XASSERT(obj);
	Object* otherObj = Pool_GetById(&GAME->objects, other->super.objId); XASSERT(otherObj);
	ComponentOffense* off = Pool_GetById(&GAME->offenses, obj->offense); XASSERT(off);
	ComponentDefense* defense = Pool_GetById(&GAME->defenses, otherObj->defense); XASSERT(defense);
	// Check if already collided
	if (off->state.projectile.alreadyCollidedThisStep) {
		return;
	}
	// Calculate damage
	defense->hp -= off->state.projectile.cfg->damage;
	if (defense->hp <= 0.0001f && defense->onDeath) {
		LOG2XV_TRC(XOK_PROJECTILE_DEATH, ID, off->super.objId, XOK_ID, ID, defense->super.objId);
		defense->onDeath(defense);
	} else {
		LOG3XV_TRC(XOK_PROJECTILE_DMG, ID, off->super.objId, XOK_ID, ID, defense->super.objId, XOK_HP, Float32, defense->hp);
		Box2DBodyApplyForceToCenter(other->body, Vec2F_Mul(Vec2F_Normalize(Box2DBodyGetLinearVelocity(phy->body)), 5000.0f), true);
	}
	off->state.projectile.alreadyCollidedThisStep = true;
	Game_DeleteList_Add(phy->super.objId);
}

int ObjectProjectile_InitFromCfg(Object* obj, const CfgProjectile *cfg, ID originatorId, Vec2F position, Vec2F direction) {
	direction = Vec2F_Normalize(direction);
	XERR_REFLECT(Object_Init(obj, position, false));

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
	Box2DBodySetLinearVelocity(phy->body, Vec2F_Mul(direction, cfg->speed));
	phy->onCollision = Bullet_onCollision;
	
	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = cfg->texture->textureRect;
	gfx->center_px = cfg->texture->objCenter_px;
	gfx->angle = Vec2F_AngleRads(direction);

	ComponentOffense* off = Object_AddOffense(obj);
	off->originator = originatorId;
	off->state.projectile.cfg = cfg;
	off->state.projectile.ttl = cfg->ttl;
	off->state.projectile.alreadyCollidedThisStep = false;

	return 0;
}
