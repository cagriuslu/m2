#include "../Object.h"
#include "../Box2DUtils.h"
#include "../Game.h"

static void ObjectExplosive_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId);
	ComponentPhysique* phy;
	ComponentOffense* offense;
	if (obj && (phy = Object_GetPhysique(obj)) && (offense = Object_GetOffense(obj))) {
		if (phy->body) {
			Box2DBodySetLinearSpeed(phy->body, offense->state.explosive.cfg->projectileSpeed);
		}

		offense->state.projectile.ttl -= GAME->deltaTicks / 1000.0f;
		if (offense->state.projectile.ttl <= 0) {
			// TODO explode
		}
	}
}

static void ObjectExplosive_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	// TODO
}

XErr ObjectExplosive_InitFromCfg(Object* obj, const CfgExplosive* cfg, ID originatorId, Vec2F position, Vec2F direction) {
	direction = Vec2F_Normalize(direction);
	REFLECT_ERROR(Object_Init(obj, position, false));

	ComponentMonitor* el = Object_AddMonitor(obj);
	el->prePhysics = ObjectExplosive_prePhysics;

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

	return XOK;
}
