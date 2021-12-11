#include "Object.h"
#include "Main.h"
#include "Pool.h"
#include <string.h>

int Object_Init(Object* obj, Vec2F position, bool initProperties) {
	memset(obj, 0, sizeof(Object));
	obj->position = position;
	if (initProperties) {
		obj->properties = calloc(1, sizeof(ObjectProperties));
		return 0;
	} else {
		return 0;
	}
}

void Object_Term(Object* obj) {
	if (obj->eventListener) {
		ComponentEventListener* el = Pool_GetById(&GAME->eventListeners, obj->eventListener);
		EventListenerComponent_Term(el);
		Pool_Unmark(&GAME->eventListeners, el);
	}
	if (obj->physics) {
		ComponentPhysics* phy = Pool_GetById(&GAME->physics, obj->physics);
		PhysicsComponent_Term(phy);
		Pool_Unmark(&GAME->physics, phy);
	}
	if (obj->graphics) {
		InsertionList_Remove(&GAME->drawList, obj->graphics);
		ComponentGraphics* gfx = Pool_GetById(&GAME->graphics, obj->graphics);
		GraphicsComponent_Term(gfx);
		Pool_Unmark(&GAME->graphics, gfx);
	}
	if (obj->terrainGraphics) {
		ComponentGraphics* gfx = Pool_GetById(&GAME->terrainGraphics, obj->terrainGraphics);
		GraphicsComponent_Term(gfx);
		Pool_Unmark(&GAME->terrainGraphics, gfx);
	}
	if (obj->defense) {
		ComponentDefense* def = Pool_GetById(&GAME->defenses, obj->defense);
		ComponentDefense_Term(def);
		Pool_Unmark(&GAME->defenses, def);
	}
	if (obj->offenseProjectile) {
		ComponentOffense* off = Pool_GetById(&GAME->offenses, obj->offenseProjectile);
		ComponentOffense_Term(off);
		Pool_Unmark(&GAME->offenses, off);
	}
	if (obj->offenseMelee) {
		ComponentOffense* off = Pool_GetById(&GAME->offenses, obj->offenseMelee);
		ComponentOffense_Term(off);
		Pool_Unmark(&GAME->offenses, off);
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
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentEventListener* el = Pool_Mark(&GAME->eventListeners, NULL, &obj->eventListener);
	EventListenerComponent_Init(el, objectId);
	if (outId) {
		outId[0] = obj->eventListener;
	}
	return el;
}

ComponentPhysics* Object_AddPhysics(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentPhysics* phy = Pool_Mark(&GAME->physics, NULL, &obj->physics);
	PhysicsComponent_Init(phy, objectId);
	if (outId) {
		outId[0] = obj->physics;
	}
	return phy;
}

ComponentGraphics* Object_AddGraphics(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentGraphics* gfx = Pool_Mark(&GAME->graphics, NULL, &obj->graphics);
	GraphicsComponent_Init(gfx, objectId);
	InsertionList_Insert(&GAME->drawList, obj->graphics);
	if (outId) {
		outId[0] = obj->graphics;
	}
	return gfx;
}

ComponentGraphics* Object_AddTerrainGraphics(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentGraphics* gfx = Pool_Mark(&GAME->terrainGraphics, NULL, &obj->terrainGraphics);
	GraphicsComponent_Init(gfx, objectId);
	if (outId) {
		outId[0] = obj->terrainGraphics;
	}
	return gfx;
}

ComponentDefense* Object_AddDefense(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentDefense* def = Pool_Mark(&GAME->defenses, NULL, &obj->defense);
	ComponentDefense_Init(def, objectId);
	if (outId) {
		outId[0] = obj->defense;
	}
	return def;
}

ComponentOffense* Object_AddOffenseProjectile(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentOffense* off = Pool_Mark(&GAME->offenses, NULL, &obj->offenseProjectile);
	ComponentOffense_Init(off, objectId);
	if (outId) {
		outId[0] = obj->offenseProjectile;
	}
	return off;
}

ComponentOffense* Object_AddOffenseMelee(Object* obj, ID* outId) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentOffense* off = Pool_Mark(&GAME->offenses, NULL, &obj->offenseMelee);
	ComponentOffense_Init(off, objectId);
	if (outId) {
		outId[0] = obj->offenseMelee;
	}
	return off;
}
