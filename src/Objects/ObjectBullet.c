#include "../Object.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include "../Item.h"
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

		ComponentOffense* offense = FindOffenseProjectileOfObject(obj);
		if (offense) {
			offense->ttl -= DeltaTicks();
			if (offense->ttl <= 0) {
				DeleteObject(obj);
			}
		}
	}
}

static void Bullet_onCollision(ComponentPhysics* phy, ComponentPhysics* other) {
	Level* level = CurrentLevel();
	Object* obj = Bucket_GetById(&level->objects, phy->super.objId);
	Object* otherObj = Bucket_GetById(&level->objects, other->super.objId);
	if (obj && obj->offenseProjectile && otherObj && otherObj->defense) {
		ComponentOffense* offense = Bucket_GetById(&level->offenses, obj->offenseProjectile);
		ComponentDefense* defense = Bucket_GetById(&level->defenses, otherObj->defense);
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

int ObjectBullet_Init(Object* obj, Vec2F position, Vec2F direction, ItemType projectileType, ComponentOffense* copyOffense) {
	direction = Vec2F_Normalize(direction);
	PROPAGATE_ERROR(Object_Init(obj, position, false));

	ComponentEventListener* el = Object_AddEventListener(obj, NULL);
	el->prePhysics = Bullet_prePhysics;

	ID phyId = 0;
	ComponentPhysics* phy = Object_AddPhysics(obj, &phyId);
	phy->body = Box2DUtils_CreateBulletSensor(
		phyId,
		position,
		CATEGORY_PLAYER_BULLET,
		0.167f, // Radius
		0.0f, // Mass
		0.0f // Damping
	);
	Box2DBodySetLinearVelocity(phy->body, Vec2F_Mul(direction, 10.0f)); // Give initial velocity
	phy->onCollision = Bullet_onCollision;
	
	ComponentGraphics* gfx = Object_AddGraphics(obj, NULL);
	gfx->txAngle = ANGLE(direction);
	switch (projectileType) {
		case ITEMTYP_GUN:
			gfx->txSrc = (SDL_Rect){ 4 * TILE_WIDTH, 4 * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
			gfx->txCenter = (Vec2F){ 1.5f, 0.5f };
			break;
		case ITEMTYP_RIFLE:
			gfx->txSrc = (SDL_Rect){ 5 * TILE_WIDTH, 4 * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
			gfx->txCenter = (Vec2F){ 3.5f, 0.5f };
			break;
		case ITEMTYP_BOW:
			gfx->txSrc = (SDL_Rect){ 3 * TILE_WIDTH, 4 * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
			gfx->txCenter = (Vec2F){ 2.5f, 0.5f };
			break;
		default:
			break;
	}

	ComponentOffense* off = Object_AddOffenseProjectile(obj, NULL);
	ComponentOffense_CopyExceptSuper(off, copyOffense);

	return 0;
}
