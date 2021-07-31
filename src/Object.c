#include "Object.h"
#include "Main.h"
#include "Bucket.h"
#include <string.h>

int ObjectProperties_Init(ObjectProperties* props) {
	memset(props, 0, sizeof(ObjectProperties));
	return 0;
}

void ObjectProperties_Term(ObjectProperties* props) {
	memset(props, 0, sizeof(ObjectProperties));
}

int Object_Init(Object* obj, Vec2F position, bool initProperties) {
	memset(obj, 0, sizeof(Object));
	obj->position = position;
	if (initProperties) {
		obj->properties = malloc(sizeof(ObjectProperties));
		return ObjectProperties_Init(obj->properties);
	} else {
		return 0;
	}
}

void Object_Term(Object* obj) {
	if (obj->eventListener) {
		ComponentEventListener* el = Bucket_GetById(&CurrentLevel()->eventListeners, obj->eventListener);
		EventListenerComponent_Term(el);
		Bucket_Unmark(&CurrentLevel()->eventListeners, el);
	}
	if (obj->physics) {
		ComponentPhysics* phy = Bucket_GetById(&CurrentLevel()->physics, obj->physics);
		PhysicsComponent_Term(phy);
		Bucket_Unmark(&CurrentLevel()->physics, phy);
	}
	if (obj->graphics) {
		InsertionList_Remove(&CurrentLevel()->drawList, obj->graphics);
		ComponentGraphics* gfx = Bucket_GetById(&CurrentLevel()->graphics, obj->graphics);
		GraphicsComponent_Term(gfx);
		Bucket_Unmark(&CurrentLevel()->graphics, gfx);
	}
	if (obj->terrainGraphics) {
		ComponentGraphics* gfx = Bucket_GetById(&CurrentLevel()->terrainGraphics, obj->terrainGraphics);
		GraphicsComponent_Term(gfx);
		Bucket_Unmark(&CurrentLevel()->terrainGraphics, gfx);
	}
	if (obj->defense) {
		ComponentDefense* def = Bucket_GetById(&CurrentLevel()->defenses, obj->defense);
		ComponentDefense_Term(def);
		Bucket_Unmark(&CurrentLevel()->defenses, def);
	}
	if (obj->offenseProjectile) {
		ComponentOffense* off = Bucket_GetById(&CurrentLevel()->offenses, obj->offenseProjectile);
		ComponentOffense_Term(off);
		Bucket_Unmark(&CurrentLevel()->offenses, off);
	}
	if (obj->offenseMelee) {
		ComponentOffense* off = Bucket_GetById(&CurrentLevel()->offenses, obj->offenseMelee);
		ComponentOffense_Term(off);
		Bucket_Unmark(&CurrentLevel()->offenses, off);
	}
	if (obj->lightSource) {
		ComponentLightSource* light = Bucket_GetById(&CurrentLevel()->lightSources, obj->lightSource);
		ComponentLightSource_Term(light);
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

ComponentEventListener* Object_AddEventListener(Object* obj, ID *outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentEventListener* el = Bucket_Mark(&CurrentLevel()->eventListeners, NULL, &obj->eventListener);
	EventListenerComponent_Init(el, objectId);
	if (outId) {
		outId[0] = obj->eventListener;
	}
	return el;
}

ComponentPhysics* Object_AddPhysics(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentPhysics* phy = Bucket_Mark(&CurrentLevel()->physics, NULL, &obj->physics);
	PhysicsComponent_Init(phy, objectId);
	if (outId) {
		outId[0] = obj->physics;
	}
	return phy;
}

ComponentGraphics* Object_AddGraphics(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentGraphics* gfx = Bucket_Mark(&CurrentLevel()->graphics, NULL, &obj->graphics);
	GraphicsComponent_Init(gfx, objectId);
	InsertionList_Insert(&CurrentLevel()->drawList, obj->graphics);
	if (outId) {
		outId[0] = obj->graphics;
	}
	return gfx;
}

ComponentGraphics* Object_AddTerrainGraphics(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentGraphics* gfx = Bucket_Mark(&CurrentLevel()->terrainGraphics, NULL, &obj->terrainGraphics);
	GraphicsComponent_Init(gfx, objectId);
	if (outId) {
		outId[0] = obj->terrainGraphics;
	}
	return gfx;
}

ComponentDefense* Object_AddDefense(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentDefense* def = Bucket_Mark(&CurrentLevel()->defenses, NULL, &obj->defense);
	ComponentDefense_Init(def, objectId);
	if (outId) {
		outId[0] = obj->defense;
	}
	return def;
}

ComponentOffense* Object_AddOffenseProjectile(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentOffense* off = Bucket_Mark(&CurrentLevel()->offenses, NULL, &obj->offenseProjectile);
	ComponentOffense_Init(off, objectId);
	if (outId) {
		outId[0] = obj->offenseProjectile;
	}
	return off;
}

ComponentOffense* Object_AddOffenseMelee(Object* obj, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentOffense* off = Bucket_Mark(&CurrentLevel()->offenses, NULL, &obj->offenseMelee);
	ComponentOffense_Init(off, objectId);
	if (outId) {
		outId[0] = obj->offenseMelee;
	}
	return off;
}

ComponentLightSource* Object_AddLightSource(Object* obj, float lightBoundaryRadius, ID* outId) {
	ID objectId = Bucket_GetId(&CurrentLevel()->objects, obj);
	ComponentLightSource* light = Bucket_Mark(&CurrentLevel()->lightSources, NULL, &obj->lightSource);
	ComponentLightSource_Init(light, objectId, lightBoundaryRadius);
	if (outId) {
		outId[0] = obj->lightSource;
	}
	return light;
}

Array* Object_AddPrePhysicsStopwatches(Object* obj, unsigned stopwatchCount) {
	Array* array = Bucket_Mark(&CurrentLevel()->prePhysicsStopwatches, NULL, &obj->prePhysicsStopwatches);
	Array_Init(array, sizeof(unsigned), stopwatchCount, stopwatchCount);
	for (unsigned i = 0; i < stopwatchCount; i++) {
		unsigned initialValue = 0;
		Array_Append(array, &initialValue);
	}
	return array;
}

Stopwatch* Object_GetPrePhysicsStopwatchPtr(Object* obj, unsigned stopwatchIdx) {
	if (obj->prePhysicsStopwatches) {
		Array* stopwatches = Bucket_GetById(&CurrentLevel()->prePhysicsStopwatches, obj->prePhysicsStopwatches);
		if (stopwatches) {
			return Array_Get(stopwatches, stopwatchIdx);
		}
	}
	return NULL;
}
