#include <m2/Object.hh>
#include <m2/Def.hh>
#include <m2/Game.hh>
#include <m2/Box2DUtils.hh>
#include "../ARPG_Component.hh"
#include "../ARPG_Cfg.hh"

#define SWING_SPEED (15.0f)

static void Sword_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); M2ASSERT(obj);
	ComponentOffense* offense = Object_GetOffense(obj); M2ASSERT(offense);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(offense->data);

	offData->state.melee.ttl_s -= GAME->deltaTicks_ms / 1000.0f;
	if (offData->state.melee.ttl_s <= 0) {
		Game_DeleteList_Add(el->super.objId);
	}
}

static void Sword_postPhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); M2ASSERT(obj);
	ComponentPhysique* phy = Object_GetPhysique(obj); M2ASSERT(phy);
	ComponentGraphic* gfx = Object_GetGraphic(obj); M2ASSERT(gfx);
	ComponentOffense* off = Object_GetOffense(obj); M2ASSERT(off);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(off->data);

	Object* originator = static_cast<Object *>(Pool_GetById(&GAME->objects, offData->originator));
	// Make sure originator is still alive
	if (originator) {
		Box2DBodySetTransform(phy->body, originator->position, Box2DBodyGetAngle(phy->body));
	}
	gfx->angle = Box2DBodyGetAngle(phy->body);
}

static void Sword_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	LOG_DEBUG("Collision");
	Object* obj = static_cast<Object *>(Pool_GetById(&GAME->objects, phy->super.objId)); M2ASSERT(obj);
	Object* otherObj = static_cast<Object *>(Pool_GetById(&GAME->objects, other->super.objId)); M2ASSERT(otherObj);
	ComponentOffense* offense = static_cast<ComponentOffense *>(Pool_GetById(&GAME->offenses,
																			 obj->offense)); M2ASSERT(offense);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(offense->data);
	ComponentDefense* defense = static_cast<ComponentDefense *>(Pool_GetById(&GAME->defenses,
																			 otherObj->defense)); M2ASSERT(defense);
	ARPG_ComponentDefense *defenseData = AS_ARPG_COMPONENTDEFENSE(defense->data);

	// Calculate damage
	defenseData->hp -= offData->state.melee.cfg->damage;
	if (defenseData->hp <= 0.0001f && defenseData->onDeath) {
		LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, offense->super.objId, M2_ID, ID, defense->super.objId);
		defenseData->onDeath(defense);
	} else {
		LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, offense->super.objId, M2_ID, ID, defense->super.objId, M2_HP, Float32, defenseData->hp);
		Box2DBodyApplyForceToCenter(other->body, (otherObj->position - obj->position).normalize() * 15000.0f, true);
		if (defenseData->onHit) {
			defenseData->onHit(defense);
		}
	}
}

int ObjectMelee_InitFromCfg(Object* obj, const CfgMelee *cfg, ID originatorId, m2::vec2f position, m2::vec2f direction) {
	M2ERR_REFLECT(Object_Init(obj, position));

	const float theta = direction.angle_rads(); // Convert direction to angle
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
		m2::vec2f{1.25f, 0.1667f}, // boxDims
		m2::vec2f{0.5833f, 0.0f}, // boxCenterOffset
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
	gfx->textureRect = ARPG_CFG_SPRITES[cfg->spriteIndex].textureRect;
	gfx->center_px = ARPG_CFG_SPRITES[cfg->spriteIndex].objCenter_px;
	gfx->angle = Box2DBodyGetAngle(phy->body);

	ComponentOffense* off = Object_AddOffense(obj);
	ARPG_ComponentOffense *offData = AS_ARPG_COMPONENTOFFENSE(off->data);
	offData->originator = originatorId;
	offData->state.melee.cfg = cfg;
	offData->state.melee.ttl_s = cfg->ttl_s;
	
	return 0;
}
