#include "../Blueprint.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include <math.h>
#include <stdio.h>
#include <assert.h>

#define ANGLE(v2fDir) atan2f((v2fDir).y, (v2fDir).x)

typedef struct _BulletData {
	Vec2F direction;
} BulletData;
#define AsBulletData(ptr) ((BulletData*) (ptr))

static void Bullet_prePhysics(GameObject* obj) {
	Box2DBodyApplyForceToCenter(obj->body, Vec2FMul(AsBulletData(obj->privData)->direction, 1.0f), true); // TODO adjust force
}

static void Bullet_onCollision(GameObject* obj, Object* otherObj) {
	(void)obj;
	fprintf(stderr, "Hit something\n");

	int objTyp = otherObj->type;
	if (IS_TILE(objTyp)) {

	} else if (IS_ENEMY(objTyp)) {

	}
}

static void Bullet_deinit(GameObject* obj) {
	if (obj) {
		if (obj->body) {
			Box2DWorldDestroyBody(CurrentWorld(), obj->body);
		}
		if (obj->privData) {
			free(obj->privData);
		}
	}
}

int BlueprintBulletInit(GameObject* obj, Vec2F position, Vec2F direction) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->privData = malloc(sizeof(BulletData));
	assert(obj->privData);
	AsBulletData(obj->privData)->direction = Vec2FNormalize(direction);

	obj->super.type = OBJECT_BULLET_BASIC;
	obj->pos = position;
	obj->angle = ANGLE(direction);
	obj->prePhysics = Bullet_prePhysics;
	obj->onCollision = Bullet_onCollision;
	obj->txSrc = (SDL_Rect){ 4 * TILE_WIDTH, 4 * TILE_WIDTH, TILE_WIDTH, TILE_WIDTH };
	obj->body = Box2DUtilsCreateBulletSensor(
		obj,
		position,
		PLAYER_BULLET_CATEGORY,
		0.167f, // Radius
		0.01f, // Mass
		10.0f // Damping
	);
	obj->deinit = Bullet_deinit;
	return 0;
}