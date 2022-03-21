#include <m2/object/Object.hh>
#include "m2/Def.hh"
#include "m2/Game.hh"
#include "m2/Box2DUtils.hh"
#include <game/component.hh>
#include <game/ARPG_Cfg.hh>

#define SWING_SPEED (15.0f)

static void Sword_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId);
	game::component_offense* offense = Object_GetOffense(obj);
    offense->state.melee.ttl_s -= GAME.deltaTicks_ms / 1000.0f;
	if (offense->state.melee.ttl_s <= 0) {
		Game_DeleteList_Add(el->super.objId);
	}
}

static void Sword_postPhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId);
	ComponentPhysique* phy = Object_GetPhysique(obj);
	ComponentGraphic* gfx = Object_GetGraphic(obj);
	game::component_offense* off = Object_GetOffense(obj);

	Object* originator = GAME.objects.get(off->originator);
	float angle = phy->body->GetAngle();
	// Make sure originator is still alive
	if (originator) {
		phy->body->SetTransform(static_cast<b2Vec2>(originator->position), angle);
	}
	
	gfx->angle = angle;
}

static void Sword_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	LOG_DEBUG("Collision");
	Object* obj = GAME.objects.get(phy->super.objId);
	Object* otherObj = GAME.objects.get(other->super.objId);
    game::component_offense* off = GAME.offenses.get(obj->offense);
    game::component_defense* def = GAME.defenses.get(otherObj->defense);

	// Calculate damage
    def->hp -= off->state.melee.cfg->damage;
	if (def->hp <= 0.0001f && def->onDeath) {
		LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off->super.objId, M2_ID, ID, def->super.objId);
        def->onDeath(def);
	} else {
		LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off->super.objId, M2_ID, ID, def->super.objId, M2_HP, Float32, def->hp);
		auto direction = (otherObj->position - obj->position).normalize();
		auto force = direction * 15000.0f;
		other->body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
		if (def->onHit) {
            def->onHit(def);
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
		GAME.physics.get_id(phy),
		false, // isDisk
		true, // isDynamic
		position,
		false, // allowSleep
		true, // isBullet
		true, // isSensor
		originatorId == GAME.playerId ? CATEGORY_PLAYER_MELEE_WEAPON : CATEGORY_ENEMY_MELEE_WEAPON, // category
		0, // mask
		m2::vec2f{1.25f, 0.1667f}, // boxDims
		m2::vec2f{0.5833f, 0.0f}, // boxCenterOffset
		0.0f, // boxAngle
		NAN, // diskRadius
		1.0f, // mass
		0.0f, // linearDamping
		false // fixedRotation
	);
	phy->body->SetTransform(static_cast<b2Vec2>(position), startAngle);
	phy->body->SetAngularVelocity(-SWING_SPEED);
	phy->onCollision = Sword_onCollision;

	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = ARPG_CFG_SPRITES[cfg->spriteIndex].textureRect;
	gfx->center_px = ARPG_CFG_SPRITES[cfg->spriteIndex].objCenter_px;
	gfx->angle = phy->body->GetAngle();

	game::component_offense* off = Object_AddOffense(obj);
    off->originator = originatorId;
    off->state.melee.cfg = cfg;
    off->state.melee.ttl_s = cfg->ttl_s;
	
	return 0;
}
