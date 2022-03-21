#include <m2/object/Object.hh>
#include "m2/Game.hh"
#include "m2/Box2DUtils.hh"
#include "m2/Def.hh"
#include <game/component.hh>
#include <game/ARPG_Cfg.hh>

static void Bullet_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId); M2ASSERT(obj);
	ComponentPhysique* phy = Object_GetPhysique(obj); M2ASSERT(phy);
	game::component_offense* offense = Object_GetOffense(obj); M2ASSERT(offense);
	m2::vec2f curr_direction = m2::vec2f{ phy->body->GetLinearVelocity() }.normalize();
	phy->body->SetLinearVelocity(static_cast<b2Vec2>(curr_direction * offense->state.projectile.cfg->speed_mps));

    offense->state.projectile.ttl_s -= GAME.deltaTicks_ms / 1000.0f;
	if (offense->state.projectile.ttl_s <= 0) {
		Game_DeleteList_Add(el->super.objId);
	}
}

static void Bullet_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	Object* obj = GAME.objects.get(phy->super.objId);
	Object* otherObj = GAME.objects.get(other->super.objId);
	game::component_offense* off = GAME.offenses.get(obj->offense);
	game::component_defense* def = GAME.defenses.get(otherObj->defense);
	// Check if already collided
	if (off->state.projectile.alreadyCollidedThisStep) {
		return;
	}
    off->state.projectile.alreadyCollidedThisStep = true;
	// Calculate damage
    def->hp -= ACCURACY(off->state.projectile.cfg->damage, off->state.projectile.cfg->damageAccuracy);
	if (def->hp <= 0.0001f && def->onDeath) {
		LOG_TRACE_M2VV(M2_PROJECTILE_DEATH, ID, off->super.objId, M2_ID, ID, def->super.objId);
        def->onDeath(def);
	} else {
		LOG_TRACE_M2VVV(M2_PROJECTILE_DMG, ID, off->super.objId, M2_ID, ID, def->super.objId, M2_HP, Float32, def->hp);
		auto direction = m2::vec2f{ phy->body->GetLinearVelocity() }.normalize();
		auto force = direction * 5000.0f;
		other->body->ApplyForceToCenter(static_cast<b2Vec2>(force), true);
		if (def->onHit) {
            def->onHit(def);
		}
	}
	Game_DeleteList_Add(phy->super.objId);
}

int ObjectProjectile_InitFromCfg(Object* obj, const CfgProjectile *cfg, ID originatorId, m2::vec2f position, m2::vec2f direction) {
	M2ERR_REFLECT(Object_Init(obj, position));
	direction = direction.normalize();

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = Bullet_prePhysics;

	ComponentPhysique* phy = Object_AddPhysique(obj);
	phy->body = Box2DUtils_CreateBulletSensor(
		GAME.physics.get_id(phy),
		position,
		CATEGORY_PLAYER_BULLET,
		0.167f, // Radius
		0.0f, // Mass
		0.0f // Damping
	);
	phy->body->SetLinearVelocity(static_cast<b2Vec2>(direction * cfg->speed_mps));
	phy->onCollision = Bullet_onCollision;
	
	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = ARPG_CFG_SPRITES[cfg->spriteIndex].textureRect;
	gfx->center_px = ARPG_CFG_SPRITES[cfg->spriteIndex].objCenter_px;
	gfx->angle = direction.angle_rads();

	game::component_offense* off = Object_AddOffense(obj);
    off->originator = originatorId;
    off->state.projectile.cfg = cfg;
    off->state.projectile.ttl_s = ACCURACY(cfg->ttl_s, cfg->ttlAccuracy);
    off->state.projectile.alreadyCollidedThisStep = false;

	return 0;
}
