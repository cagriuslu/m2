#include "../Object.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include "../Log.h"
#include <math.h>
#include <stdio.h>

#define ANGLE(v2fDir) atan2f((v2fDir).y, (v2fDir).x)

static void Bullet_prePhysics(ComponentEventListener* el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj && obj->physics) {
		ComponentPhysics* phy = FindPhysicsOfObject(obj);
		if (phy && phy->body) {
			const Vec2F direction = Vec2F_Normalize(Box2DBodyGetLinearVelocity(phy->body));
			Box2DBodySetLinearVelocity(phy->body, Vec2F_Mul(direction, 20.0f));
		}

		ComponentOffense* offense = FindOffenseOfObject(obj);
		if (offense) {
			offense->ttl -= GAME->deltaTicks / 1000.0f;
			if (offense->ttl <= 0) {
				DeleteObject(obj);
			}
		}
	}
}

static void Bullet_onCollision(ComponentPhysics* phy, ComponentPhysics* other) {
	Object* obj = Pool_GetById(&GAME->objects, phy->super.objId);
	Object* otherObj = Pool_GetById(&GAME->objects, other->super.objId);
	if (obj && obj->offense && otherObj && otherObj->defense) {
		ComponentOffense* offense = Pool_GetById(&GAME->offenses, obj->offense);
		ComponentDefense* defense = Pool_GetById(&GAME->defenses, otherObj->defense);
		if (offense && defense) {
			// Calculate damage
			defense->hp -= offense->hp;

			const Vec2F direction = Vec2F_Normalize(Box2DBodyGetLinearVelocity(phy->body));
			Box2DBodyApplyForceToCenter(other->body, Vec2F_Mul(direction, 5000.0f), true);
			LOG_DBG("Hit");
		}
	}
	DeleteObject(obj);
}

int ObjectProjectile_InitFromCfg(Object* obj, const CfgProjectile *cfg, ID originatorId, Vec2F position, Vec2F direction) {
	direction = Vec2F_Normalize(direction);
	REFLECT_ERROR(Object_Init(obj, position, false));

	ComponentEventListener* el = Object_AddEventListener(obj);
	el->prePhysics = Bullet_prePhysics;

	ComponentPhysics* phy = Object_AddPhysics(obj);
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
	
	ComponentGraphics* gfx = Object_AddGraphics(obj);
	gfx->txAngle = ANGLE(direction);
	gfx->txSrc = (SDL_Rect){ 4 * GAME->tileWidth, 4 * GAME->tileWidth, GAME->tileWidth, GAME->tileWidth };
	gfx->txCenter = (Vec2F){ 1.5f, 0.5f };

	ComponentOffense* off = Object_AddOffense(obj);
	off->originator = originatorId;
	off->hp = cfg->damage;
	off->ttl = cfg->ttl;

	return 0;
}
