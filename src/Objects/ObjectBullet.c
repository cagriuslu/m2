#include "../Object.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include "../Log.h"
#include <math.h>
#include <stdio.h>

#define ANGLE(v2fDir) atan2f((v2fDir).y, (v2fDir).x)

static void Bullet_prePhysics(EventListenerComponent* el) {
	Object* obj = FindObjectOfComponent(el);
	if (obj && obj->physics) {
		PhysicsComponent* phy = FindPhysicsOfObject(obj);
		if (phy && phy->body) {
			const Vec2F direction = Vec2FNormalize(Box2DBodyGetLinearVelocity(phy->body));
			Box2DBodySetLinearVelocity(phy->body, Vec2FMul(direction, 10.0f));
		}

		ComponentOffense* offense = FindOffenseProjectileOfObject(obj);
		if (offense) {
			offense->ttl -= DeltaTicks();
			if (offense->ttl <= 0) {
				DeleteObject(obj);
			}
		}
	}
}

static void Bullet_onCollision(PhysicsComponent* phy, PhysicsComponent* other) {
	Level* level = CurrentLevel();
	Object* obj = Bucket_GetById(&level->objects, phy->super.objId);
	Object* otherObj = Bucket_GetById(&level->objects, other->super.objId);
	if (obj && obj->offenseProjectile && otherObj && otherObj->defense) {
		ComponentOffense* offense = Bucket_GetById(&level->offenses, obj->offenseProjectile);
		ComponentDefense* defense = Bucket_GetById(&level->defenses, otherObj->defense);
		if (offense && defense) {
			// Calculate damage
			defense->hp -= offense->hp;

			const Vec2F direction = Vec2FNormalize(Box2DBodyGetLinearVelocity(phy->body));
			Box2DBodyApplyForceToCenter(other->body, Vec2FMul(direction, 5000.0f), true);
			LOG_DBG("Hit");
		}
	}
	DeleteObject(obj);
}

int ObjectBulletInit(Object* obj, Vec2F position, Vec2F direction, ComponentOffense** outOffense) {
	direction = Vec2FNormalize(direction);
	PROPAGATE_ERROR(ObjectInit(obj, position));

	EventListenerComponent* el = ObjectAddEventListener(obj, NULL);
	el->prePhysics = Bullet_prePhysics;

	ID phyId = 0;
	PhysicsComponent* phy = ObjectAddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateBulletSensor(
		phyId,
		position,
		CATEGORY_PLAYER_BULLET,
		0.167f, // Radius
		0.0f, // Mass
		0.0f // Damping
	);
	Box2DBodySetLinearVelocity(phy->body, Vec2FMul(direction, 10.0f)); // Give initial velocity
	phy->onCollision = Bullet_onCollision;
	
	GraphicsComponent* gfx = ObjectAddGraphics(obj, NULL);
	gfx->txAngle = ANGLE(direction);
	gfx->txSrc = (SDL_Rect){ 4 * TILE_WIDTH, 4 * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };

	ComponentOffense* off = ObjectAddOffenseProjectile(obj, NULL);
	if (outOffense) {
		*outOffense = off;
	}

	return 0;
}
