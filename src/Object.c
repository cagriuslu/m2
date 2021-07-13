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
		EventListenerComponent* el = Bucket_GetById(&CurrentLevel()->eventListeners, obj->eventListener);
		EventListenerComponentDeinit(el);
		Bucket_Unmark(&CurrentLevel()->eventListeners, el);
	}
	if (obj->physics) {
		PhysicsComponent* phy = Bucket_GetById(&CurrentLevel()->physics, obj->physics);
		PhysicsComponentDeinit(phy);
		Bucket_Unmark(&CurrentLevel()->physics, phy);
	}
	if (obj->graphics) {
		InsertionListRemove(&CurrentLevel()->drawList, obj->graphics);
		GraphicsComponent* gfx = Bucket_GetById(&CurrentLevel()->graphics, obj->graphics);
		GraphicsComponentDeinit(gfx);
		Bucket_Unmark(&CurrentLevel()->graphics, gfx);
	}
	if (obj->terrainGraphics) {
		GraphicsComponent* gfx = Bucket_GetById(&CurrentLevel()->terrainGraphics, obj->terrainGraphics);
		GraphicsComponentDeinit(gfx);
		Bucket_Unmark(&CurrentLevel()->terrainGraphics, gfx);
	}
	if (obj->defense) {
		ComponentDefense* def = Bucket_GetById(&CurrentLevel()->defenses, obj->defense);
		ComponentDefenseDeinit(def);
		Bucket_Unmark(&CurrentLevel()->defenses, def);
	}
	if (obj->offenseProjectile) {
		ComponentOffense* off = Bucket_GetById(&CurrentLevel()->offenses, obj->offenseProjectile);
		ComponentOffenseDeinit(off);
		Bucket_Unmark(&CurrentLevel()->offenses, off);
	}
	if (obj->offenseMelee) {
		ComponentOffense* off = Bucket_GetById(&CurrentLevel()->offenses, obj->offenseMelee);
		ComponentOffenseDeinit(off);
		Bucket_Unmark(&CurrentLevel()->offenses, off);
	}
	if (obj->lightSource) {
		ComponentLightSource* light = Bucket_GetById(&CurrentLevel()->lightSources, obj->lightSource);
		ComponentLightSourceDeinit(light);
		Bucket_Unmark(&CurrentLevel()->lightSources, light);
	}
	if (obj->ai) {
		AI_Term(obj->ai);
	}
	if (obj->prePhysicsStopwatches) {
		Array* stopwatches = Bucket_GetById(&CurrentLevel()->prePhysicsStopwatches, obj->prePhysicsStopwatches);
		Array_Term(stopwatches);
		Bucket_Unmark(&CurrentLevel()->prePhysicsStopwatches, stopwatches);
	}
	memset(obj, 0, sizeof(Object));
}

EventListenerComponent* ObjectAddEventListener(Object* obj, ID *outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	EventListenerComponent* el = Bucket_Mark(&CurrentLevel()->eventListeners, NULL, &obj->eventListener);
	EventListenerComponentInit(el, objectId);
	if (outId) {
		outId[0] = obj->eventListener;
	}
	return el;
}

PhysicsComponent* ObjectAddPhysics(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	PhysicsComponent* phy = Bucket_Mark(&CurrentLevel()->physics, NULL, &obj->physics);
	PhysicsComponentInit(phy, objectId);
	if (outId) {
		outId[0] = obj->physics;
	}
	return phy;
}

GraphicsComponent* ObjectAddGraphics(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	GraphicsComponent* gfx = Bucket_Mark(&CurrentLevel()->graphics, NULL, &obj->graphics);
	GraphicsComponentInit(gfx, objectId);
	InsertionListInsert(&CurrentLevel()->drawList, obj->graphics);
	if (outId) {
		outId[0] = obj->graphics;
	}
	return gfx;
}

GraphicsComponent* ObjectAddTerrainGraphics(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	GraphicsComponent* gfx = Bucket_Mark(&CurrentLevel()->terrainGraphics, NULL, &obj->terrainGraphics);
	GraphicsComponentInit(gfx, objectId);
	if (outId) {
		outId[0] = obj->terrainGraphics;
	}
	return gfx;
}

ComponentDefense* ObjectAddDefense(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentDefense* def = Bucket_Mark(&CurrentLevel()->defenses, NULL, &obj->defense);
	ComponentDefenseInit(def, objectId);
	if (outId) {
		outId[0] = obj->defense;
	}
	return def;
}

ComponentOffense* ObjectAddOffenseProjectile(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentOffense* off = Bucket_Mark(&CurrentLevel()->offenses, NULL, &obj->offenseProjectile);
	ComponentOffenseInit(off, objectId);
	if (outId) {
		outId[0] = obj->offenseProjectile;
	}
	return off;
}

ComponentOffense* ObjectAddOffenseMelee(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentOffense* off = Bucket_Mark(&CurrentLevel()->offenses, NULL, &obj->offenseMelee);
	ComponentOffenseInit(off, objectId);
	if (outId) {
		outId[0] = obj->offenseMelee;
	}
	return off;
}

ComponentLightSource* ObjectAddLightSource(Object* obj, float lightBoundaryRadius, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentLightSource* light = Bucket_Mark(&CurrentLevel()->lightSources, NULL, &obj->lightSource);
	ComponentLightSourceInit(light, objectId, lightBoundaryRadius);
	if (outId) {
		outId[0] = obj->lightSource;
	}
	return light;
}

Array* ObjectAddPrePhysicsStopwatches(Object* obj, unsigned stopwatchCount) {
	Array* array = Bucket_Mark(&CurrentLevel()->prePhysicsStopwatches, NULL, &obj->prePhysicsStopwatches);
	Array_Init(array, sizeof(unsigned), stopwatchCount, stopwatchCount);
	for (unsigned i = 0; i < stopwatchCount; i++) {
		unsigned initialValue = 0;
		Array_Append(array, &initialValue);
	}
	return array;
}
