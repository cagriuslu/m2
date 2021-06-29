#include "Object.h"
#include "Main.h"
#include "Bucket.h"
#include <string.h>

int ObjectInit(Object* obj, Vec2F position) {
	memset(obj, 0, sizeof(Object));
	obj->position = position;
	return 0;
}

void ObjectDeinit(Object* obj) {
	if (obj->eventListener) {
		EventListenerComponent* el = BucketGetById(&CurrentLevel()->eventListeners, obj->eventListener);
		EventListenerComponentDeinit(el);
		BucketUnmark(&CurrentLevel()->eventListeners, el);
	}
	if (obj->physics) {
		PhysicsComponent* phy = BucketGetById(&CurrentLevel()->physics, obj->physics);
		PhysicsComponentDeinit(phy);
		BucketUnmark(&CurrentLevel()->physics, phy);
	}
	if (obj->graphics) {
		InsertionListRemove(&CurrentLevel()->drawList, obj->graphics);
		GraphicsComponent* gfx = BucketGetById(&CurrentLevel()->graphics, obj->graphics);
		GraphicsComponentDeinit(gfx);
		BucketUnmark(&CurrentLevel()->graphics, gfx);
	}
	if (obj->terrainGraphics) {
		GraphicsComponent* gfx = BucketGetById(&CurrentLevel()->terrainGraphics, obj->terrainGraphics);
		GraphicsComponentDeinit(gfx);
		BucketUnmark(&CurrentLevel()->terrainGraphics, gfx);
	}
	if (obj->defense) {
		ComponentDefense* def = BucketGetById(&CurrentLevel()->defenses, obj->defense);
		ComponentDefenseDeinit(def);
		BucketUnmark(&CurrentLevel()->defenses, def);
	}
	if (obj->offense) {
		ComponentOffense* off = BucketGetById(&CurrentLevel()->offenses, obj->offense);
		ComponentOffenseDeinit(off);
		BucketUnmark(&CurrentLevel()->offenses, off);
	}
	if (obj->lightSource) {
		ComponentLightSource* light = BucketGetById(&CurrentLevel()->lightSources, obj->lightSource);
		ComponentLightSourceDeinit(light);
		BucketUnmark(&CurrentLevel()->lightSources, light);
	}
	if (obj->ai) {
		AIDeinit(obj->ai);
	}
	if (obj->prePhysicsStopwatches) {
		Array* stopwatches = BucketGetById(&CurrentLevel()->prePhysicsStopwatches, obj->prePhysicsStopwatches);
		ArrayDeinit(stopwatches);
		BucketUnmark(&CurrentLevel()->prePhysicsStopwatches, stopwatches);
	}
	memset(obj, 0, sizeof(Object));
}

EventListenerComponent* ObjectAddEventListener(Object* obj, uint64_t *outId) {
	uint64_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	EventListenerComponent* el = BucketMark(&CurrentLevel()->eventListeners, NULL, &obj->eventListener);
	EventListenerComponentInit(el, objectId);
	if (outId) {
		outId[0] = obj->eventListener;
	}
	return el;
}

PhysicsComponent* ObjectAddPhysics(Object* obj, uint64_t* outId) {
	uint64_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	PhysicsComponent* phy = BucketMark(&CurrentLevel()->physics, NULL, &obj->physics);
	PhysicsComponentInit(phy, objectId);
	if (outId) {
		outId[0] = obj->physics;
	}
	return phy;
}

GraphicsComponent* ObjectAddGraphics(Object* obj, uint64_t* outId) {
	uint64_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	GraphicsComponent* gfx = BucketMark(&CurrentLevel()->graphics, NULL, &obj->graphics);
	GraphicsComponentInit(gfx, objectId);
	InsertionListInsert(&CurrentLevel()->drawList, obj->graphics);
	if (outId) {
		outId[0] = obj->graphics;
	}
	return gfx;
}

GraphicsComponent* ObjectAddTerrainGraphics(Object* obj, uint64_t* outId) {
	uint64_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	GraphicsComponent* gfx = BucketMark(&CurrentLevel()->terrainGraphics, NULL, &obj->terrainGraphics);
	GraphicsComponentInit(gfx, objectId);
	if (outId) {
		outId[0] = obj->terrainGraphics;
	}
	return gfx;
}

ComponentDefense* ObjectAddDefense(Object* obj, uint64_t* outId) {
	uint64_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	ComponentDefense* def = BucketMark(&CurrentLevel()->defenses, NULL, &obj->defense);
	ComponentDefenseInit(def, objectId);
	if (outId) {
		outId[0] = obj->defense;
	}
	return def;
}

ComponentOffense* ObjectAddOffense(Object* obj, uint64_t* outId) {
	uint64_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	ComponentOffense* off = BucketMark(&CurrentLevel()->offenses, NULL, &obj->offense);
	ComponentOffenseInit(off, objectId);
	if (outId) {
		outId[0] = obj->offense;
	}
	return off;
}

ComponentLightSource* ObjectAddLightSource(Object* obj, float lightBoundaryRadius, uint64_t* outId) {
	uint64_t objectId = BucketGetId(&CurrentLevel()->objects, obj);
	ComponentLightSource* light = BucketMark(&CurrentLevel()->lightSources, NULL, &obj->lightSource);
	ComponentLightSourceInit(light, objectId, lightBoundaryRadius);
	if (outId) {
		outId[0] = obj->lightSource;
	}
	return light;
}

Array* ObjectAddPrePhysicsStopwatches(Object* obj, unsigned stopwatchCount) {
	Array* array = BucketMark(&CurrentLevel()->prePhysicsStopwatches, NULL, &obj->prePhysicsStopwatches);
	ArrayInit(array, sizeof(unsigned), stopwatchCount, stopwatchCount);
	for (unsigned i = 0; i < stopwatchCount; i++) {
		unsigned initialValue = 0;
		ArrayAppend(array, &initialValue);
	}
	return array;
}
