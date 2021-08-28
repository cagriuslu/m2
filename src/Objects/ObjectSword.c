#include "../Object.h"
#include "../Log.h"
#include "../Main.h"
#include "../Error.h"
#include "../Box2DUtils.h"

#define SWING_SPEED (15.0f)

static void Sword_prePhysics(ComponentEventListener* el) {
	Object* obj = FindObjectOfComponent(el);
	ComponentOffense* offense = FindOffenseMeleeOfObject(obj);
	offense->ttl -= DeltaTicks();
	if (offense->ttl <= 0) {
		DeleteObject(obj);
	}
}

static void Sword_postPhysics(ComponentEventListener* el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj && obj->physics && obj->graphics && obj->offenseMelee) {
		ComponentPhysics* phy = FindPhysicsOfObject(obj);
		ComponentGraphics* gfx = FindGraphicsOfObject(obj);
		ComponentOffense* off = FindOffenseMeleeOfObject(obj);
		if (phy && phy->body && gfx && off && off->originator) {
			Object* originator = Bucket_GetById(&CurrentLevel()->objects, off->originator);
			if (originator) {
				Box2DBodySetTransform(phy->body, originator->position, Box2DBodyGetAngle(phy->body));
			}
			gfx->txAngle = Box2DBodyGetAngle(phy->body);
		}
	}
}

static void Sword_onCollision(ComponentPhysics* phy, ComponentPhysics* other) {
	LOG_DBG("Collision");
	Level* level = CurrentLevel();
	Object* obj = Bucket_GetById(&level->objects, phy->super.objId);
	Object* otherObj = Bucket_GetById(&level->objects, other->super.objId);
	if (obj && obj->offenseMelee && otherObj && otherObj->defense) {
		ComponentOffense* offense = Bucket_GetById(&level->offenses, obj->offenseMelee);
		ComponentDefense* defense = Bucket_GetById(&level->defenses, otherObj->defense);
		if (offense && defense) {
			// Calculate damage
			defense->hp -= offense->hp;

			const Vec2F direction = Vec2F_Normalize(Vec2F_Sub(otherObj->position, obj->position));
			Box2DBodyApplyForceToCenter(other->body, Vec2F_Mul(direction, 15000.0f), true);
			LOG_DBG("Hit");
		}
	}
}

int ObjectSword_Init(Object* obj, Vec2F originatorPosition, ComponentOffense* originatorOffense, bool isEnemy, Vec2F direction, uint32_t ticks) {
	PROPAGATE_ERROR(Object_Init(obj, originatorPosition, false));

	const float theta = Vec2F_AngleRads(direction); // Convert direction to angle
	const float startAngle = theta + SWING_SPEED * (ticks / 1000.0f / 2.0f);

	ComponentEventListener* el = Object_AddEventListener(obj, NULL);
	el->prePhysics = Sword_prePhysics;
	el->postPhysics = Sword_postPhysics;

	ID phyId = 0;
	ComponentPhysics* phy = Object_AddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateBody(
		phyId,
		false, // isDisk
		true, // isDynamic
		originatorPosition,
		false, // allowSleep
		true, // isBullet
		true, // isSensor
		isEnemy ? CATEGORY_ENEMY_MELEE_WEAPON : CATEGORY_PLAYER_MELEE_WEAPON, // category
		0, // mask
		(Vec2F) {1.25f, 0.1667f}, // boxDims
		(Vec2F) {0.5833f, 0.0f}, // boxCenterOffset
		0.0f, // boxAngle
		NAN, // diskRadius
		1.0f, // mass
		0.0f, // linearDamping
		false // fixedRotation
	);
	Box2DBodySetTransform(phy->body, originatorPosition, startAngle);
	Box2DBodySetAngularVelocity(phy->body, -SWING_SPEED);
	phy->onCollision = Sword_onCollision;

	ComponentGraphics* gfx = Object_AddGraphics(obj, NULL);
	gfx->txAngle = Box2DBodyGetAngle(phy->body);
	gfx->txSrc = (SDL_Rect){ 6 * TILE_WIDTH, 4 * TILE_WIDTH, 2 * TILE_WIDTH, TILE_WIDTH };
	gfx->txCenter = (Vec2F){ -14.0f, 0.0f };

	ComponentOffense* off = Object_AddOffenseMelee(obj, NULL);
	ComponentOffense_CopyExceptSuper(off, originatorOffense);
	off->ttl = ticks;
	
	return 0;
}
