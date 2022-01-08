#include "../Object.h"
#include "../Game.h"
#include "../Box2DUtils.h"
#include "../Log.h"
#include <math.h>
#include <stdio.h>

static void Bullet_prePhysics(ComponentMonitor* el) {
	Object* obj = Game_FindObjectById(el->super.objId);
	ComponentPhysique* phy;
	ComponentOffense* offense;
	if (obj && (phy = Object_GetPhysique(obj)) && (offense = Object_GetOffense(obj))) {
		if (phy->body) {
			const Vec2F direction = Vec2F_Normalize(Box2DBodyGetLinearVelocity(phy->body));
			Box2DBodySetLinearVelocity(phy->body, Vec2F_Mul(direction, 20.0f));
		}

		offense->ttl -= GAME->deltaTicks / 1000.0f;
		if (offense->ttl <= 0) {
			Game_DeleteList_Add(el->super.objId);
		}
	}
}

static void Bullet_onCollision(ComponentPhysique* phy, ComponentPhysique* other) {
	Object* obj = Pool_GetById(&GAME->objects, phy->super.objId);
	Object* otherObj = Pool_GetById(&GAME->objects, other->super.objId);
	if (obj && obj->offense && otherObj && otherObj->defense) {
		ComponentOffense* offense = Pool_GetById(&GAME->offenses, obj->offense);
		ComponentDefense* defense = Pool_GetById(&GAME->defenses, otherObj->defense);
		if (offense && defense) {
			// Calculate damage
			defense->hp -= offense->hp;
			if (defense->hp <= 0.0001f && defense->onDeath) {
				LOG2XV_TRC(XOK_PROJECTILE_DEATH,ID,offense->super.objId,    XOK_ID,ID,defense->super.objId);
				defense->onDeath(defense);
			} else {
				LOG3XV_TRC(XOK_PROJECTILE_DMG,ID,offense->super.objId,    XOK_ID,ID,defense->super.objId,    XOK_HP,Float32,defense->hp);
				Box2DBodyApplyForceToCenter(other->body, Vec2F_Mul(Vec2F_Normalize(Box2DBodyGetLinearVelocity(phy->body)), 5000.0f), true);
			}
		}
	}
	Game_DeleteList_Add(phy->super.objId);
}

int ObjectProjectile_InitFromCfg(Object* obj, const CfgProjectile *cfg, ID originatorId, Vec2F position, Vec2F direction) {
	direction = Vec2F_Normalize(direction);
	REFLECT_ERROR(Object_Init(obj, position, false));

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
	Box2DBodySetLinearVelocity(phy->body, Vec2F_Mul(direction, 10.0f)); // Give initial velocity
	phy->onCollision = Bullet_onCollision;
	
	ComponentGraphic* gfx = Object_AddGraphic(obj);
	gfx->textureRect = cfg->texture->textureRect;
	gfx->center_px = cfg->texture->objCenter_px;
	gfx->angle = Vec2F_AngleRads(direction);

	ComponentOffense* off = Object_AddOffense(obj);
	off->originator = originatorId;
	off->hp = cfg->damage;
	off->ttl = cfg->ttl;

	return 0;
}
