#include "m2/Object.hh"
#include "m2/Game.hh"
#include "m2/Pool.hh"
#include "m2/String.hh"

M2Err Object_Init(Object* obj, m2::vec2f position) {
	memset(obj, 0, sizeof(Object));
	obj->position = position;
	return M2OK;
}

ComponentMonitor* Object_GetMonitor(Object* obj) {
	return static_cast<ComponentMonitor *>(Pool_GetById(&GAME->monitors, obj->monitor));
}

ComponentPhysique* Object_GetPhysique(Object* obj) {
	return static_cast<ComponentPhysique *>(Pool_GetById(&GAME->physics, obj->physique));
}

ComponentGraphic* Object_GetGraphic(Object* obj) {
	return static_cast<ComponentGraphic *>(Pool_GetById(&GAME->graphics, obj->graphic));
}

ComponentGraphic* Object_GetTerrainGraphic(Object* obj) {
	return static_cast<ComponentGraphic *>(Pool_GetById(&GAME->terrainGraphics, obj->terrainGraphic));
}

ComponentLight* Object_GetLight(Object* obj) {
	return static_cast<ComponentLight *>(Pool_GetById(&GAME->lights, obj->light));
}

ComponentDefense* Object_GetDefense(Object* obj) {
	return static_cast<ComponentDefense *>(Pool_GetById(&GAME->defenses, obj->defense));
}

ComponentOffense* Object_GetOffense(Object* obj) {
	return static_cast<ComponentOffense *>(Pool_GetById(&GAME->offenses, obj->offense));
}

ComponentMonitor* Object_AddMonitor(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentMonitor* el = static_cast<ComponentMonitor *>(Pool_Mark(&GAME->monitors, NULL, &obj->monitor));
	ComponentMonitor_Init(el, objectId);
	return el;
}

ComponentPhysique* Object_AddPhysique(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentPhysique* phy = static_cast<ComponentPhysique *>(Pool_Mark(&GAME->physics, NULL, &obj->physique));
	ComponentPhysique_Init(phy, objectId);
	return phy;
}

ComponentGraphic* Object_AddGraphic(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentGraphic* gfx = static_cast<ComponentGraphic *>(Pool_Mark(&GAME->graphics, NULL, &obj->graphic));
	ComponentGraphic_Init(gfx, objectId);
	InsertionList_Insert(&GAME->drawList, obj->graphic);
	return gfx;
}

ComponentGraphic* Object_AddTerrainGraphic(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentGraphic* gfx = static_cast<ComponentGraphic *>(Pool_Mark(&GAME->terrainGraphics, NULL,
																	  &obj->terrainGraphic));
	ComponentGraphic_Init(gfx, objectId);
	return gfx;
}

ComponentLight* Object_AddLight(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentLight* lig = static_cast<ComponentLight *>(Pool_Mark(&GAME->lights, NULL, &obj->light));
	ComponentLight_Init(lig, objectId);
	return lig;
}

ComponentDefense* Object_AddDefense(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentDefense* def = static_cast<ComponentDefense *>(Pool_Mark(&GAME->defenses, NULL, &obj->defense));
	ComponentDefense_Init(def, objectId);
	return def;
}

ComponentOffense* Object_AddOffense(Object* obj) {
	ID objectId = Pool_GetId(&GAME->objects, obj);
	ComponentOffense* off = static_cast<ComponentOffense *>(Pool_Mark(&GAME->offenses, NULL, &obj->offense));
	ComponentOffense_Init(off, objectId);
	return off;
}

void Object_Term(Object* obj) {
	if (obj->monitor) {
		ComponentMonitor* el = static_cast<ComponentMonitor *>(Pool_GetById(&GAME->monitors, obj->monitor));
		ComponentMonitor_Term(el);
		Pool_Unmark(&GAME->monitors, el);
	}
	if (obj->physique) {
		ComponentPhysique* phy = static_cast<ComponentPhysique *>(Pool_GetById(&GAME->physics, obj->physique));
		ComponentPhysique_Term(phy);
		Pool_Unmark(&GAME->physics, phy);
	}
	if (obj->graphic) {
		InsertionList_Remove(&GAME->drawList, obj->graphic);
		ComponentGraphic* gfx = static_cast<ComponentGraphic *>(Pool_GetById(&GAME->graphics, obj->graphic));
		ComponentGraphic_Term(gfx);
		Pool_Unmark(&GAME->graphics, gfx);
	}
	if (obj->terrainGraphic) {
		ComponentGraphic* gfx = static_cast<ComponentGraphic *>(Pool_GetById(&GAME->terrainGraphics,
																			 obj->terrainGraphic));
		ComponentGraphic_Term(gfx);
		Pool_Unmark(&GAME->terrainGraphics, gfx);
	}
	if (obj->defense) {
		ComponentDefense* def = static_cast<ComponentDefense *>(Pool_GetById(&GAME->defenses, obj->defense));
		ComponentDefense_Term(def);
		Pool_Unmark(&GAME->defenses, def);
	}
	if (obj->offense) {
		ComponentOffense* off = static_cast<ComponentOffense *>(Pool_GetById(&GAME->offenses, obj->offense));
		ComponentOffense_Term(off);
		Pool_Unmark(&GAME->offenses, off);
	}
	if (obj->data) {
		free(obj->data);
	}
	memset(obj, 0, sizeof(Object));
}
