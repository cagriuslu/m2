#include "Object.h"
#include "Game.h"
#include "Pool.h"
#include <string.h>

XErr Object_Init(Object* obj, Vec2F position, bool initExtra) {
	memset(obj, 0, sizeof(Object));
	obj->position = position;
	if (initExtra) {
		if ((obj->ex = calloc(1, sizeof(ObjectEx))) != NULL) {
			return XOK;
		} else {
			// TODO print error message
			return XERR_OUT_OF_MEMORY;
		}
	} else {
		return XOK;
	}
}

ComponentMonitor* Object_GetMonitor(Object* obj) {
	return Pool_GetById(&GAME->monitors, obj->monitor);
}

ComponentPhysique* Object_GetPhysique(Object* obj) {
	return Pool_GetById(&GAME->physics, obj->physique);
}

ComponentGraphic* Object_GetGraphic(Object* obj) {
	return Pool_GetById(&GAME->graphics, obj->graphic);
}

ComponentGraphic* Object_GetTerrainGraphic(Object* obj) {
	return Pool_GetById(&GAME->terrainGraphics, obj->terrainGraphic);
}

ComponentLight* Object_GetLight(Object* obj) {
	return Pool_GetById(&GAME->lights, obj->light);
}

ComponentDefense* Object_GetDefense(Object* obj) {
	return Pool_GetById(&GAME->defenses, obj->defense);
}

ComponentOffense* Object_GetOffense(Object* obj) {
	return Pool_GetById(&GAME->offenses, obj->offense);
}

ComponentMonitor* Object_AddMonitor(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentMonitor* el = Pool_Mark(&GAME->monitors, NULL, &obj->monitor);
	ComponentMonitor_Init(el, objectId);
	return el;
}

ComponentPhysique* Object_AddPhysique(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentPhysique* phy = Pool_Mark(&GAME->physics, NULL, &obj->physique);
	ComponentPhysique_Init(phy, objectId);
	return phy;
}

ComponentGraphic* Object_AddGraphic(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentGraphic* gfx = Pool_Mark(&GAME->graphics, NULL, &obj->graphic);
	ComponentGraphic_Init(gfx, objectId);
	InsertionList_Insert(&GAME->drawList, obj->graphic);
	return gfx;
}

ComponentGraphic* Object_AddTerrainGraphic(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentGraphic* gfx = Pool_Mark(&GAME->terrainGraphics, NULL, &obj->terrainGraphic);
	ComponentGraphic_Init(gfx, objectId);
	return gfx;
}

ComponentLight* Object_AddLight(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentLight* lig = Pool_Mark(&GAME->lights, NULL, &obj->light);
	ComponentLight_Init(lig, objectId);
	return lig;
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

void Object_Term(Object* obj) {
	if (obj->monitor) {
		ComponentMonitor* el = Pool_GetById(&GAME->monitors, obj->monitor);
		ComponentMonitor_Term(el);
		Pool_Unmark(&GAME->monitors, el);
	}
	if (obj->physique) {
		ComponentPhysique* phy = Pool_GetById(&GAME->physics, obj->physique);
		ComponentPhysique_Term(phy);
		Pool_Unmark(&GAME->physics, phy);
	}
	if (obj->graphic) {
		InsertionList_Remove(&GAME->drawList, obj->graphic);
		ComponentGraphic* gfx = Pool_GetById(&GAME->graphics, obj->graphic);
		ComponentGraphic_Term(gfx);
		Pool_Unmark(&GAME->graphics, gfx);
	}
	if (obj->terrainGraphic) {
		ComponentGraphic* gfx = Pool_GetById(&GAME->terrainGraphics, obj->terrainGraphic);
		ComponentGraphic_Term(gfx);
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
