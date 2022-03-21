#include "m2/Object.hh"
#include "m2/Game.hh"

M2Err Object_Init(Object* obj, m2::vec2f position) {
	memset(obj, 0, sizeof(Object));
	obj->position = position;
	return M2OK;
}

ComponentMonitor* Object_GetMonitor(Object* obj) {
	return GAME.monitors.get(obj->monitor);
}

ComponentPhysique* Object_GetPhysique(Object* obj) {
	return GAME.physics.get(obj->physique);
}

ComponentGraphic* Object_GetGraphic(Object* obj) {
	return GAME.graphics.get(obj->graphic);
}

ComponentGraphic* Object_GetTerrainGraphic(Object* obj) {
	return GAME.terrainGraphics.get(obj->terrainGraphic);
}

ComponentLight* Object_GetLight(Object* obj) {
	return GAME.lights.get(obj->light);
}

game::component_defense* Object_GetDefense(Object* obj) {
	return GAME.defenses.get(obj->defense);
}

game::component_offense* Object_GetOffense(Object* obj) {
	return GAME.offenses.get(obj->offense);
}

ComponentMonitor* Object_AddMonitor(Object* obj) {
	ID objectId = GAME.objects.get_id(obj);

    auto monitor_pair = GAME.monitors.alloc();
    obj->monitor = monitor_pair.second;
	ComponentMonitor_Init(&monitor_pair.first, objectId);
	return &monitor_pair.first;
}

ComponentPhysique* Object_AddPhysique(Object* obj) {
	ID objectId = GAME.objects.get_id(obj);

    auto physique_pair = GAME.physics.alloc();
    obj->physique = physique_pair.second;
	ComponentPhysique_Init(&physique_pair.first, objectId);
	return &physique_pair.first;
}

ComponentGraphic* Object_AddGraphic(Object* obj) {
	ID objectId = GAME.objects.get_id(obj);

    auto graphic_pair = GAME.graphics.alloc();
    obj->graphic = graphic_pair.second;
	ComponentGraphic_Init(&graphic_pair.first, objectId);
	InsertionList_Insert(&GAME.drawList, obj->graphic);
	return &graphic_pair.first;
}

ComponentGraphic* Object_AddTerrainGraphic(Object* obj) {
	ID objectId = GAME.objects.get_id(obj);

    auto terrain_graphic_pair = GAME.terrainGraphics.alloc();
    obj->terrainGraphic = terrain_graphic_pair.second;
	ComponentGraphic_Init(&terrain_graphic_pair.first, objectId);
	return &terrain_graphic_pair.first;
}

ComponentLight* Object_AddLight(Object* obj) {
	ID objectId = GAME.objects.get_id(obj);

    auto light_pair = GAME.lights.alloc();
    obj->light = light_pair.second;
	ComponentLight_Init(&light_pair.first, objectId);
	return &light_pair.first;
}

game::component_defense* Object_AddDefense(Object* obj) {
	ID objectId = GAME.objects.get_id(obj);

    auto defense_pair = GAME.defenses.alloc();
    obj->defense = defense_pair.second;
	defense_pair.first = game::component_defense(objectId);
	return &defense_pair.first;
}

game::component_offense* Object_AddOffense(Object* obj) {
	ID objectId = GAME.objects.get_id(obj);

    auto offense_pair = GAME.offenses.alloc();
    obj->offense = offense_pair.second;
    offense_pair.first = game::component_offense(objectId);
	return &offense_pair.first;
}

void Object_Term(Object* obj) {
	if (obj->monitor) {
        ComponentMonitor *monitor = GAME.monitors.get(obj->monitor);
        ComponentMonitor_Term(monitor);
        GAME.monitors.free(obj->monitor);
	}
	if (obj->physique) {
        ComponentPhysique *physique = GAME.physics.get(obj->physique);
        ComponentPhysique_Term(physique);
        GAME.physics.free(obj->physique);
	}
	if (obj->graphic) {
		InsertionList_Remove(&GAME.drawList, obj->graphic);
        ComponentGraphic *graphic = GAME.graphics.get(obj->graphic);
        ComponentGraphic_Term(graphic);
        GAME.graphics.free(obj->graphic);
	}
	if (obj->terrainGraphic) {
        auto* gfx = GAME.terrainGraphics.get(obj->terrainGraphic);
        ComponentGraphic_Term(gfx);
        GAME.terrainGraphics.free(obj->terrainGraphic);
	}
	if (obj->defense) {
        GAME.defenses.free(obj->defense);
	}
	if (obj->offense) {
        GAME.offenses.free(obj->offense);
	}
	if (obj->data) {
		free(obj->data);
	}
	memset(obj, 0, sizeof(Object));
}
