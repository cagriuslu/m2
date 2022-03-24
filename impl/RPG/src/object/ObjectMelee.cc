#include <m2/Object.h>
#include "m2/Def.hh"
#include "m2/Game.hh"
#include "m2/Box2DUtils.hh"
#include "impl/public/Component.hh"
#include "impl/private/ARPG_Cfg.hh"
#include "Monitor.h"

#define SWING_SPEED (15.0f)

static void Sword_prePhysics(m2::component::Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
	auto& off = obj.offense();

	off.state.melee.ttl_s -= GAME.deltaTicks_ms / 1000.0f;
	if (off.state.melee.ttl_s <= 0) {
		Game_DeleteList_Add(mon.object_id);
	}
}

static void Sword_postPhysics(m2::component::Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
	auto& phy = obj.physique();
	float angle = phy.body->GetAngle();
	auto& gfx = obj.graphic();
	auto& off = obj.offense();

	m2::Object* originator = GAME.objects.get(off.originator);
	if (originator) {
		// Make sure originator is still alive
		phy.body->SetTransform(static_cast<b2Vec2>(originator->position), angle);
	}
	gfx.angle = angle;
}

static void Sword_onCollision(component::Physique& phy, component::Physique& other) {
	LOG_DEBUG("Collision");
	auto& obj = GAME.objects[phy.object_id];
	auto& other_obj = GAME.objects[other.object_id];
	auto& off = GAME.offenses[obj.offense_id];
	auto& def = GAME.defenses[other_obj.defense_id];

	// Calculate damage
    def.hp -= off.state.melee.cfg->damage;
	if (def.hp <= 0.0001f && def.onDeath) {
		LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off.object_id, M2_ID, ID, def.object_id);
        def.onDeath(&def);
	} else {
		LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off.object_id, M2_ID, ID, def.object_id, M2_HP, Float32, def.hp);
		auto direction = (other_obj.position - obj.position).normalize();
		auto force = direction * 15000.0f;
		other.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
		if (def.onHit) {
            def.onHit(&def);
		}
	}
}

int ObjectMelee_InitFromCfg(m2::Object* obj, const CfgMelee *cfg, ID originatorId, m2::Vec2f position, m2::Vec2f direction) {
	*obj = m2::Object{position};

	const float theta = direction.angle_rads(); // Convert direction to angle
	const float startAngle = theta + SWING_SPEED * (150 / 1000.0f / 2.0f);

	auto& mon = obj->add_monitor();
	mon.prePhysics = Sword_prePhysics;
	mon.postPhysics = Sword_postPhysics;

	auto& phy = obj->add_physique();
	phy.body = Box2DUtils_CreateBody(
			obj->physique_id,
		false, // isDisk
		true, // isDynamic
		position,
		false, // allowSleep
		true, // isBullet
		true, // isSensor
		originatorId == GAME.playerId ? CATEGORY_PLAYER_MELEE_WEAPON : CATEGORY_ENEMY_MELEE_WEAPON, // category
		0, // mask
		m2::Vec2f{1.25f, 0.1667f}, // boxDims
		m2::Vec2f{0.5833f, 0.0f}, // boxCenterOffset
		0.0f, // boxAngle
		NAN, // diskRadius
		1.0f, // mass
		0.0f, // linearDamping
		false // fixedRotation
	);
	phy.body->SetTransform(static_cast<b2Vec2>(position), startAngle);
	phy.body->SetAngularVelocity(-SWING_SPEED);
	phy.onCollision = Sword_onCollision;

	auto& gfx = obj->add_graphic();
	gfx.textureRect = ARPG_CFG_SPRITES[cfg->spriteIndex].textureRect;
	gfx.center_px = ARPG_CFG_SPRITES[cfg->spriteIndex].objCenter_px;
	gfx.angle = phy.body->GetAngle();

	auto& off = obj->add_offense();
    off.originator = originatorId;
    off.state.melee.cfg = cfg;
    off.state.melee.ttl_s = cfg->ttl_s;
	
	return 0;
}
