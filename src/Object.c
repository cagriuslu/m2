#include "Object.h"
#include "Game.h"
#include "Pool.h"
#include <string.h>

int Object_Init(Object* obj, Vec2F position, bool initProperties) {
	memset(obj, 0, sizeof(Object));
	obj->position = position;
	if (initProperties) {
		obj->ex = calloc(1, sizeof(ObjectEx));
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
	if (obj->offense) {
		ComponentOffense* off = Pool_GetById(&GAME->offenses, obj->offense);
		ComponentOffense_Term(off);
		Pool_Unmark(&GAME->offenses, off);
	}
	if (obj->ex) {
		// TODO
	}
	memset(obj, 0, sizeof(Object));
}

ComponentEventListener* Object_AddEventListener(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentEventListener* el = Pool_Mark(&GAME->eventListeners, NULL, &obj->eventListener);
	EventListenerComponent_Init(el, objectId);
	return el;
}

ComponentPhysics* Object_AddPhysics(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentPhysics* phy = Pool_Mark(&GAME->physics, NULL, &obj->physics);
	PhysicsComponent_Init(phy, objectId);
	return phy;
}

ComponentGraphics* Object_AddGraphics(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentGraphics* gfx = Pool_Mark(&GAME->graphics, NULL, &obj->graphics);
	GraphicsComponent_Init(gfx, objectId);
	InsertionList_Insert(&GAME->drawList, obj->graphics);
	return gfx;
}

ComponentGraphics* Object_AddTerrainGraphics(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentGraphics* gfx = Pool_Mark(&GAME->terrainGraphics, NULL, &obj->terrainGraphics);
	GraphicsComponent_Init(gfx, objectId);
	return gfx;
}

ComponentDefense* Object_AddDefense(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentDefense* def = Pool_Mark(&GAME->defenses, NULL, &obj->defense);
	ComponentDefense_Init(def, objectId);
	return def;
}

ComponentOffense* Object_AddOffense(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentOffense* off = Pool_Mark(&GAME->offenses, NULL, &obj->offense);
	ComponentOffense_Init(off, objectId);
	return off;
}
