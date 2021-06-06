#include "../Blueprint.h"
#include "../Main.h"
#include "../Box2DUtils.h"
#include <math.h>
#include <stdio.h>

#define ANGLE(v2fDir) atan2f((v2fDir).y, (v2fDir).x)

static void Bullet_prePhysics(Object* obj) {
	Box2DBodyApplyForceToCenter(obj->body, (Vec2F) {1,1}, true);
}

static void Bullet_onCollision(Object* obj, ObjectType* otherObjType) {
	(void)obj;
	(void)otherObjType;
	fprintf(stderr, "Hit something\n");
}

static void Bullet_deinit(Object* obj) {
	Box2DWorldDestroyBody(CurrentWorld(), obj->body);
}

int BlueprintBulletInit(Object* obj, Vec2F position, Vec2F direction) {
	PROPAGATE_ERROR(ObjectInit(obj));
	obj->type = OBJECT_BULLET_BASIC;
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
		0.1f, // Mass
		1.0f // Damping
	);
	obj->deinit = Bullet_deinit;
	return 0;
}