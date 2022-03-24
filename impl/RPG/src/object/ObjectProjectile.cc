#include <m2/Object.h>
#include "m2/Game.hh"
#include "m2/Box2DUtils.hh"
#include "m2/Def.hh"
#include "impl/public/Component.hh"
#include "impl/private/ARPG_Cfg.hh"
#include "Monitor.h"

static void Bullet_prePhysics(m2::component::Monitor& mon) {
	auto& obj = GAME.objects[mon.object_id];
	auto& phy = obj.physique();
	auto& off = obj.offense();

	m2::Vec2f curr_direction = m2::Vec2f{phy.body->GetLinearVelocity() }.normalize();
	phy.body->SetLinearVelocity(static_cast<b2Vec2>(curr_direction * off.state.projectile.cfg->speed_mps));

    off.state.projectile.ttl_s -= GAME.deltaTicks_ms / 1000.0f;
	if (off.state.projectile.ttl_s <= 0) {
		Game_DeleteList_Add(mon.object_id);
	}
}

static void Bullet_onCollision(component::Physique& phy, component::Physique& other) {
	auto& obj = GAME.objects[phy.object_id];
	auto& other_obj = GAME.objects[other.object_id];
	auto& off = GAME.offenses[obj.offense_id];
	auto* def = GAME.defenses.get(other_obj.defense_id);
	if (def) {
		// Check if already collided
		if (off.state.projectile.alreadyCollidedThisStep) {
			return;
		}
		off.state.projectile.alreadyCollidedThisStep = true;
		// Calculate damage
		def->hp -= ACCURACY(off.state.projectile.cfg->damage, off.state.projectile.cfg->damageAccuracy);
		if (def->hp <= 0.0001f && def->onDeath) {
			LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off.object_id, M2_ID, ID, def->object_id);
			def->onDeath(def);
		} else {
			LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off.object_id, M2_ID, ID, def->object_id, M2_HP, Float32, def->hp);
			auto direction = m2::Vec2f{phy.body->GetLinearVelocity() }.normalize();
			auto force = direction * 5000.0f;
			other.body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
			if (def->onHit) {
				def->onHit(def);
			}
		}
		Game_DeleteList_Add(phy.object_id);
	}
}

int ObjectProjectile_InitFromCfg(m2::Object* obj, const CfgProjectile *cfg, ID originatorId, m2::Vec2f position, m2::Vec2f direction) {
	*obj = m2::Object{position};
	direction = direction.normalize();

	auto& mon = obj->add_monitor();
	mon.prePhysics = Bullet_prePhysics;

	auto& phy = obj->add_physique();
	phy.body = Box2DUtils_CreateBulletSensor(
			obj->physique_id,
			position,
			CATEGORY_PLAYER_BULLET,
			0.167f, // Radius
			0.0f, // Mass
			0.0f // Damping
	);
	phy.body->SetLinearVelocity(static_cast<b2Vec2>(direction * cfg->speed_mps));
	phy.onCollision = Bullet_onCollision;

	auto& gfx = obj->add_graphic();
	gfx.textureRect = ARPG_CFG_SPRITES[cfg->spriteIndex].textureRect;
	gfx.center_px = ARPG_CFG_SPRITES[cfg->spriteIndex].objCenter_px;
	gfx.angle = direction.angle_rads();

	auto& off = obj->add_offense();
    off.originator = originatorId;
    off.state.projectile.cfg = cfg;
    off.state.projectile.ttl_s = ACCURACY(cfg->ttl_s, cfg->ttlAccuracy);
    off.state.projectile.alreadyCollidedThisStep = false;

	return 0;
}
