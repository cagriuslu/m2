#include "../Object.h"
#include "../Def.h"
#include "../Game.h"
#include "../Box2DUtils.h"

#define SWING_SPEED (15.0f)

static void Sword_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); M2ASSERT(obj);
	ComponentOffense* offense = Object_GetOffense(obj); M2ASSERT(offense);
	offense->state.melee.ttl_s -= GAME->deltaTicks / 1000.0f;
	if (offense->state.melee.ttl_s <= 0) {
		Game_DeleteList_Add(el->super.objId);
	}
}

static void Sword_postPhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); M2ASSERT(obj);
	ComponentPhysique* phy = Object_GetPhysique(obj); M2ASSERT(phy);
	ComponentGraphic* gfx = Object_GetGraphic(obj); M2ASSERT(gfx);
	ComponentOffense* off = Object_GetOffense(obj); M2ASSERT(off);
	Object* originator = Pool_GetById(&GAME->objects, off->originator);
	// Make sure originator is still alive
	// TODO what if an object is destroyed, then a new one is created with the same ID?
	if (originator) {
		Box2DBodySetTransform(phy->body, originator->position, Box2DBodyGetAngle(phy->body));
	}
	gfx->angle = Box2DBodyGetAngle(phy->body);
}

static void Sword_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	LOG_DEBUG("Collision");
	Object* obj = Pool_GetById(&GAME->objects, phy->super.objId); M2ASSERT(obj);
	Object* otherObj = Pool_GetById(&GAME->objects, other->super.objId); M2ASSERT(otherObj);
	ComponentOffense* offense = Pool_GetById(&GAME->offenses, obj->offense); M2ASSERT(offense);
	ComponentDefense* defense = Pool_GetById(&GAME->defenses, otherObj->defense); M2ASSERT(defense);
	// Calculate damage
	defense->hp -= offense->state.melee.cfg->damage;
	if (defense->hp <= 0.0001f && defense->onDeath) {
		LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, offense->super.objId, M2_ID, ID, defense->super.objId);
		defense->onDeath(defense);
	} else {
		LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, offense->super.objId, M2_ID, ID, defense->super.objId, M2_HP, Float32, defense->hp);
		Box2DBodyApplyForceToCenter(other->body, Vec2F_Mul(Vec2F_Normalize(Vec2F_Sub(otherObj->position, obj->position)), 15000.0f), true);
		if (defense->onHit) {
			defense->onHit(defense);
		}
	}
}

int ObjectMelee_InitFromCfg(Object* obj, const CfgMelee *cfg, ID originatorId, Vec2F position, Vec2F direction) {
	M2ERR_REFLECT(Object_Init(obj, position, false));

	const float theta = Vec2F_AngleRads(direction); // Convert direction to angle
	const float startAngle = theta + SWING_SPEED * (150 / 1000.0f / 2.0f);

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = Sword_prePhysics;
	el->postPhysics = Sword_postPhysics;

	ComponentPhysique* phy = Object_AddPhysique(obj);
	phy->body = Box2DUtils_CreateBody(
		Pool_GetId(&GAME->physics, phy),
		false, // isDisk
		true, // isDynamic
		position,
		false, // allowSleep
		true, // isBullet
		true, // isSensor
		originatorId == GAME->playerId ? CATEGORY_PLAYER_MELEE_WEAPON : CATEGORY_ENEMY_MELEE_WEAPON, // category
		0, // mask
		(Vec2F) {1.25f, 0.1667f}, // boxDims
		(Vec2F) {0.5833f, 0.0f}, // boxCenterOffset
		0.0f, // boxAngle
		NAN, // diskRadius
		1.0f, // mass
		0.0f, // linearDamping
		false // fixedRotation
	);
	Box2DBodySetTransform(phy->body, position, startAngle);
	Box2DBodySetAngularVelocity(phy->body, -SWING_SPEED);
	phy->onCollision = Sword_onCollision;

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = cfg->texture->textureRect;
	gfx->center_px = cfg->texture->objCenter_px;
	gfx->angle = Box2DBodyGetAngle(phy->body);

	ComponentOffense* off = Object_AddOffense(obj);
	off->originator = originatorId;
	off->state.melee.cfg = cfg;
	off->state.melee.ttl_s = cfg->ttl_s;
	
	return 0;
}
