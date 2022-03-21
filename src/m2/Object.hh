#ifndef OBJECT_H
#define OBJECT_H

#include <game/component.hh>
#include <m2/Component.hh>
#include <m2/Automaton.hh>
#include <m2/Cfg.hh>
#include <m2/vec2f.hh>
#include <memory>
#include <stdint.h>

struct ObjectData {
    virtual ~ObjectData();
};

/// Basis of all objects in the game.
/// 
/// How to decide if a component should reside in Pool or data?
/// If the component is iterated by the Main Game Loop => Pool
/// If the component is created and destroyed rapidly => Pool
/// Else => Data
struct Object {
	m2::vec2f position;
	// Components
	ID monitor;
	ID physique;
	ID graphic;
	ID terrainGraphic;
	ID light;
	ID defense;
	ID offense;
	// Extra
	void *data;
    std::unique_ptr<ObjectData> data_new;
};

M2Err Object_Init(Object* obj, m2::vec2f position);

ComponentMonitor* Object_GetMonitor(Object* obj);
ComponentPhysique* Object_GetPhysique(Object* obj);
ComponentGraphic* Object_GetGraphic(Object* obj);
ComponentGraphic* Object_GetTerrainGraphic(Object* obj);
ComponentLight* Object_GetLight(Object* obj);
game::component_defense* Object_GetDefense(Object* obj);
game::component_offense* Object_GetOffense(Object* obj);

ComponentMonitor* Object_AddMonitor(Object* obj);
ComponentPhysique* Object_AddPhysique(Object* obj);
ComponentGraphic* Object_AddGraphic(Object* obj);
ComponentGraphic* Object_AddTerrainGraphic(Object* obj);
ComponentLight* Object_AddLight(Object* obj);
game::component_defense* Object_AddDefense(Object* obj);
game::component_offense* Object_AddOffense(Object* obj);

void Object_Term(Object* obj);

// Different Object types
int ObjectTile_InitFromCfg(Object* obj, CfgSpriteIndex spriteIndex, m2::vec2f position);
M2Err ObjectCamera_Init(Object* obj);
int ObjectPointer_Init(Object* obj);

#endif
