#include "../Object.h"
#include "../Log.h"
#include "../Main.h"
#include "../Error.h"
#include "../Box2DUtils.h"

#define SWING_SPEED (15.0f)

static void Sword_prePhysics(EventListenerComponent* el) {
	Object* obj = FindObjectOfComponent(el);
	ComponentOffense* offense = FindOffenseOfObject(obj);
	offense->ticksLeft -= DeltaTicks();
	if (offense->ticksLeft <= 0) {
		DeleteObject(obj);
	}
}

static void Sword_postPhysics(EventListenerComponent* el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj && obj->physics && obj->graphics && obj->offense) {
		PhysicsComponent* phy = FindPhysicsOfObject(obj);
		GraphicsComponent* gfx = FindGraphicsOfObject(obj);
		ComponentOffense* off = FindOffenseOfObject(obj);
		if (phy && phy->body && gfx && off && off->originator) {
			Object* originator = BucketGetById(&CurrentLevel()->objects, off->originator);
			if (originator) {
				Box2DBodySetTransform(phy->body, originator->position, Box2DBodyGetAngle(phy->body));
			}
			gfx->txAngle = Box2DBodyGetAngle(phy->body);
		}
	}
}

static void Sword_onCollision(PhysicsComponent* phy, PhysicsComponent* other) {
	Level* level = CurrentLevel();
	Object* obj = BucketGetById(&level->objects, phy->super.objId);
	Object* otherObj = BucketGetById(&level->objects, other->super.objId);
	if (obj && obj->offense && otherObj && otherObj->defense) {
		ComponentOffense* offense = BucketGetById(&level->offenses, obj->offense);
		ComponentDefense* defense = BucketGetById(&level->defenses, otherObj->defense);
		if (offense && defense) {
			// Calculate damage
			defense->hp -= 3 * offense->hp;

			const Vec2F direction = Vec2FNormalize(Vec2FSub(otherObj->position, obj->position));
			Box2DBodyApplyForceToCenter(other->body, Vec2FMul(direction, 15000.0f), true);
			LOG_DBG("Hit");
		}
	}
}

int ObjectSwordInit(Object* obj, Vec2F originatorPosition, ComponentOffense* originatorOffense, Vec2F direction, uint32_t ticks) {
	PROPAGATE_ERROR(ObjectInit(obj, originatorPosition));

	const float theta = Vec2FAngleRads(direction); // Convert direction to angle
	const float startAngle = theta + SWING_SPEED * (ticks / 1000.0f / 2.0f);

	EventListenerComponent* el = ObjectAddEventListener(obj, NULL);
	el->prePhysics = Sword_prePhysics;
	el->postPhysics = Sword_postPhysics;

	uint64_t phyId = 0;
	PhysicsComponent* phy = ObjectAddPhysics(obj, &phyId);
	phy->body = Box2DUtilsCreateBody(
		phyId,
		false, // isDisk
		true, // isDynamic
		originatorPosition,
		false, // allowSleep
		true, // isBullet
		true, // isSensor
		PLAYER_MELEE_WEAPON_CATEGORY, // category
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

	GraphicsComponent* gfx = ObjectAddGraphics(obj, NULL);
	gfx->txAngle = Box2DBodyGetAngle(phy->body);
	gfx->txSrc = (SDL_Rect){ 6 * TILE_WIDTH, 4 * TILE_WIDTH, 2 * TILE_WIDTH, TILE_WIDTH };
	gfx->txCenter = (Vec2F){ -14.0f, 0.0f };

	ComponentOffense* off = ObjectAddOffense(obj, NULL);
	ComponentOffenseCopyExceptSuper(off, originatorOffense);
	off->ticksLeft = ticks;
	
	return 0;
}
