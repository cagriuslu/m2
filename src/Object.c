#include "Object.h"
#include "Main.h"
#include <string.h>

int ObjectInit(Object* obj, Vec2F position) {
	memset(obj, 0, sizeof(Object));
	obj->position = position;
	return 0;
}

void ObjectDeinit(Object* obj) {
	// TODO look for components and delete them
	memset(obj, 0, sizeof(Object));
}

EventListenerComponent* ObjectAddAndInitEventListener(Object* obj, uint32_t *outId) {
	uint32_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	EventListenerComponent* el = BucketMark(&CurrentLevel()->eventListeners, NULL, &obj->eventListener);
	EventListenerComponentInit(el, objectId);
	if (outId) {
		outId[0] = obj->eventListener;
	}
	return el;
}

PhysicsComponent* ObjectAddAndInitPhysics(Object* obj, uint32_t* outId) {
	uint32_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	PhysicsComponent* phy = BucketMark(&CurrentLevel()->physics, NULL, &obj->physics);
	PhysicsComponentInit(phy, objectId);
	if (outId) {
		outId[0] = obj->physics;
	}
	return phy;
}

GraphicsComponent* ObjectAddAndInitGraphics(Object* obj, uint32_t* outId) {
	uint32_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	GraphicsComponent* gfx = BucketMark(&CurrentLevel()->graphics, NULL, &obj->graphics);
	GraphicsComponentInit(gfx, objectId);
	InsertionListInsert(&CurrentLevel()->drawList, &obj->graphics);
	if (outId) {
		outId[0] = obj->graphics;
	}
	return gfx;
}

GraphicsComponent* ObjectAddAndInitTerrainGraphics(Object* obj, uint32_t* outId) {
	uint32_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	GraphicsComponent* gfx = BucketMark(&CurrentLevel()->terrainGraphics, NULL, &obj->terrainGraphics);
	GraphicsComponentInit(gfx, objectId);
	if (outId) {
		outId[0] = obj->terrainGraphics;
	}
	return gfx;
}
