#include "Object.h"
#include "Main.h"
#include "Pool.h"
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
		ComponentEventListener* el = Pool_GetById(&CurrentLevel()->eventListeners, obj->eventListener);
		EventListenerComponent_Term(el);
		Pool_Unmark(&CurrentLevel()->eventListeners, el);
	}
	if (obj->physics) {
		ComponentPhysics* phy = Pool_GetById(&CurrentLevel()->physics, obj->physics);
		PhysicsComponent_Term(phy);
		Pool_Unmark(&CurrentLevel()->physics, phy);
	}
	if (obj->graphics) {
		InsertionList_Remove(&CurrentLevel()->drawList, obj->graphics);
		ComponentGraphics* gfx = Pool_GetById(&CurrentLevel()->graphics, obj->graphics);
		GraphicsComponent_Term(gfx);
		Pool_Unmark(&CurrentLevel()->graphics, gfx);
	}
	if (obj->terrainGraphics) {
		ComponentGraphics* gfx = Pool_GetById(&CurrentLevel()->terrainGraphics, obj->terrainGraphics);
		GraphicsComponent_Term(gfx);
		Pool_Unmark(&CurrentLevel()->terrainGraphics, gfx);
	}
	if (obj->defense) {
		ComponentDefense* def = Pool_GetById(&CurrentLevel()->defenses, obj->defense);
		ComponentDefense_Term(def);
		Pool_Unmark(&CurrentLevel()->defenses, def);
	}
	if (obj->offenseProjectile) {
		ComponentOffense* off = Pool_GetById(&CurrentLevel()->offenses, obj->offenseProjectile);
		ComponentOffense_Term(off);
		Pool_Unmark(&CurrentLevel()->offenses, off);
	}
	if (obj->offenseMelee) {
		ComponentOffense* off = Pool_GetById(&CurrentLevel()->offenses, obj->offenseMelee);
		ComponentOffense_Term(off);
		Pool_Unmark(&CurrentLevel()->offenses, off);
	}
	if (obj->properties) {
		if (obj->properties->character) {
			// TODO: who should terminate the Character?
		}
		if (obj->properties->ai) {
			AI_Term(obj->properties->ai);
			free(obj->properties->ai);
		}
	}
	memset(obj, 0, sizeof(Object));
}

ComponentEventListener* Object_AddEventListener(Object* obj, ID *outId) {
	ID objectId = Pool_GetId(&CurrentLevel()->objects, obj);
	ComponentEventListener* el = Pool_Mark(&CurrentLevel()->eventListeners, NULL, &obj->eventListener);
	EventListenerComponent_Init(el, objectId);
	if (outId) {
		outId[0] = obj->eventListener;
	}
	return el;
}

ComponentPhysics* Object_AddPhysics(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&CurrentLevel()->objects, obj);
	ComponentPhysics* phy = Pool_Mark(&CurrentLevel()->physics, NULL, &obj->physics);
	PhysicsComponent_Init(phy, objectId);
	if (outId) {
		outId[0] = obj->physics;
	}
	return phy;
}

ComponentGraphics* Object_AddGraphics(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&CurrentLevel()->objects, obj);
	ComponentGraphics* gfx = Pool_Mark(&CurrentLevel()->graphics, NULL, &obj->graphics);
	GraphicsComponent_Init(gfx, objectId);
	InsertionList_Insert(&CurrentLevel()->drawList, obj->graphics);
	if (outId) {
		outId[0] = obj->graphics;
	}
	return gfx;
}

ComponentGraphics* Object_AddTerrainGraphics(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&CurrentLevel()->objects, obj);
	ComponentGraphics* gfx = Pool_Mark(&CurrentLevel()->terrainGraphics, NULL, &obj->terrainGraphics);
	GraphicsComponent_Init(gfx, objectId);
	if (outId) {
		outId[0] = obj->terrainGraphics;
	}
	return gfx;
}

ComponentDefense* Object_AddDefense(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&CurrentLevel()->objects, obj);
	ComponentDefense* def = Pool_Mark(&CurrentLevel()->defenses, NULL, &obj->defense);
	ComponentDefense_Init(def, objectId);
	if (outId) {
		outId[0] = obj->defense;
	}
	return def;
}

ComponentOffense* Object_AddOffenseProjectile(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&CurrentLevel()->objects, obj);
	ComponentOffense* off = Pool_Mark(&CurrentLevel()->offenses, NULL, &obj->offenseProjectile);
	ComponentOffense_Init(off, objectId);
	if (outId) {
		outId[0] = obj->offenseProjectile;
	}
	return off;
}

ComponentOffense* Object_AddOffenseMelee(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&CurrentLevel()->objects, obj);
	ComponentOffense* off = Pool_Mark(&CurrentLevel()->offenses, NULL, &obj->offenseMelee);
	ComponentOffense_Init(off, objectId);
	if (outId) {
		outId[0] = obj->offenseMelee;
	}
	return off;
}
