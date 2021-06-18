#include "../Object.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>

#define ANGLE(v2fDir) atan2f((v2fDir).y, (v2fDir).x)

static void Bullet_prePhysics(EventListenerComponent* el) {
	Object* obj = BucketGetById(&CurrentLevel()->objects, el->super.objId);
	if (obj && obj->physics) {
		PhysicsComponent* phy = BucketGetById(&CurrentLevel()->physics, obj->physics);
		if (phy && phy->body) {
			Vec2F direction = Vec2FNormalize(Box2DBodyGetLinearVelocity(phy->body));
			Box2DBodySetLinearVelocity(phy->body, Vec2FMul(direction, 10.0f));
		}
	}
}

static void Bullet_onCollision(PhysicsComponent* phy, PhysicsComponent* other) {
	Level* level = CurrentLevel();
	Object* obj = BucketGetById(&level->objects, phy->super.objId);
	Object* otherObj = BucketGetById(&level->objects, other->super.objId);
	if (obj && obj->offense && otherObj && otherObj->defense) {
		ComponentOffense* offense = BucketGetById(&level->offenses, obj->offense);
		ComponentDefense* defense = BucketGetById(&level->defenses, otherObj->defense);
		if (offense && defense) {
			// Calculate damage
			defense->hp -= offense->hp;
			
			Vec2F direction = Vec2FNormalize(Box2DBodyGetLinearVelocity(phy->body));
			Box2DBodyApplyForceToCenter(other->body, Vec2FMul(direction, 500.0f), true);
			fprintf(stderr, "Defense HP %d\n", defense->hp);
		}
	}
	DeleteObject(obj);
}

int ObjectBulletInit(Object* obj, Vec2F position, Vec2F direction, ComponentOffense** outOffense) {
	direction = Vec2FNormalize(direction);
	PROPAGATE_ERROR(ObjectInit(obj, position));

	EventListenerComponent* el = ObjectAddAndInitEventListener(obj, NULL);
	el->prePhysics = Bullet_prePhysics;

	uint32_t phyId = 0;
	PhysicsComponent* phy = ObjectAddAndInitPhysics(obj, &phyId);
	phy->body = Box2DUtilsCreateBulletSensor(
		phyId,
		position,
		PLAYER_BULLET_CATEGORY,
		0.167f, // Radius
		0.01f, // Mass
		10.0f // Damping
	);
	Box2DBodySetLinearVelocity(phy->body, Vec2FMul(direction, 10.0f)); // Give initial velocity
	phy->onCollision = Bullet_onCollision;
	
	GraphicsComponent* gfx = ObjectAddAndInitGraphics(obj, NULL);
	gfx->txAngle = ANGLE(direction);
	gfx->txSrc = (SDL_Rect){ 4 * TILE_WIDTH, 4 * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };

	ComponentOffense* off = ObjectAddAndInitOffense(obj, NULL);
	if (outOffense) {
		*outOffense = off;
	}

	return 0;
}
