#include "../Object.h"
#include "../Log.h"
#include "../Main.h"
#include "../Error.h"
#include "../Box2DUtils.h"

#define SWING_SPEED (15.0f)

static void Sword_prePhysics(ComponentEventListener* el) {
	Object* obj = FindObjectOfComponent(el);
	ComponentOffense* offense = FindOffenseOfObject(obj);
	offense->ttl -= GAME->deltaTicks;
	if (offense->ttl <= 0) {
		DeleteObject(obj);
	}
}

static void Sword_postPhysics(ComponentEventListener* el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj && obj->physics && obj->graphics && obj->offense) {
		ComponentPhysics* phy = FindPhysicsOfObject(obj);
		ComponentGraphics* gfx = FindGraphicsOfObject(obj);
		ComponentOffense* off = FindOffenseOfObject(obj);
		if (phy && phy->body && gfx && off && off->originator) {
			Object* originator = Pool_GetById(&GAME->objects, off->originator);
			if (originator) {
				Box2DBodySetTransform(phy->body, originator->position, Box2DBodyGetAngle(phy->body));
			}
			gfx->txAngle = Box2DBodyGetAngle(phy->body);
		}
	}
}

static void Sword_onCollision(ComponentPhysics* phy, ComponentPhysics* other) {
	LOG_DBG("Collision");
	Object* obj = Pool_GetById(&GAME->objects, phy->super.objId);
	Object* otherObj = Pool_GetById(&GAME->objects, other->super.objId);
	if (obj && obj->offense && otherObj && otherObj->defense) {
		ComponentOffense* offense = Pool_GetById(&GAME->offenses, obj->offense);
		ComponentDefense* defense = Pool_GetById(&GAME->defenses, otherObj->defense);
		if (offense && defense) {
			// Calculate damage
			defense->hp -= offense->hp;

			const Vec2F direction = Vec2F_Normalize(Vec2F_Sub(otherObj->position, obj->position));
			Box2DBodyApplyForceToCenter(other->body, Vec2F_Mul(direction, 15000.0f), true);
			LOG_DBG("Hit");
		}
	}
}

int ObjectMelee_InitFromCfg(Object* obj, const CfgMelee *cfg, ID originatorId, Vec2F position, Vec2F direction) {
	REFLECT_ERROR(Object_Init(obj, position, false));

	const float theta = Vec2F_AngleRads(direction); // Convert direction to angle
	const float startAngle = theta + SWING_SPEED * (150 / 1000.0f / 2.0f);

	ComponentEventListener* el = Object_AddEventListener(obj);
	el->prePhysics = Sword_prePhysics;
	el->postPhysics = Sword_postPhysics;

	ComponentPhysics* phy = Object_AddPhysics(obj);
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

	ComponentGraphics* gfx = Object_AddGraphics(obj);
	gfx->txAngle = Box2DBodyGetAngle(phy->body);
	gfx->txSrc = (SDL_Rect){ 6 * GAME->tileWidth, 4 * GAME->tileWidth, 2 * GAME->tileWidth, GAME->tileWidth };
	gfx->txCenter = (Vec2F){ -14.0f, 0.0f };

	ComponentOffense* off = Object_AddOffense(obj);
	off->originator = originatorId;
	off->hp = cfg->damage;
	off->ttl = 150;
	
	return 0;
}
