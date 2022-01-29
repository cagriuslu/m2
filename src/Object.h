#ifndef OBJECT_H
#define OBJECT_H

#include "Component.h"
#include "Automaton.h"
#include "AI.h"
#include "Array.h"
#include "Cfg.h"
#include "Vec2F.h"
#include <stdint.h>

typedef union _ObjectEx {
	struct {
		Vec2F offset;
	} camera;
	struct {
		CharacterState characterState;
		Automaton charAnimationAutomaton;
	} player;
	struct {
		CharacterState characterState;
		Automaton charAnimationAutomaton;
		AiState aiState;
		Automaton aiAutomaton;
		float onHitColorModTtl;
		AI* ai;
	} enemy;
} ObjectEx;
/// Basis of all objects in the game.
/// 
/// How to decide if a component should reside in Pool or be held as a property?
/// If the component is accessed by the Main Game Loop => Pool
/// If the component is created and destroyed rapidly => Pool
/// Others => Extra
typedef struct _Object {
	Vec2F position; // in world coordinates
	// Components
	ID monitor;
	ID physique;
	ID graphic;
	ID terrainGraphic;
	ID light;
	ID defense;
	ID offense;
	ObjectEx* ex;
} Object;

XErr Object_Init(Object* obj, Vec2F position, bool initExtra);

ComponentMonitor* Object_GetMonitor(Object* obj);
ComponentPhysique* Object_GetPhysique(Object* obj);
ComponentGraphic* Object_GetGraphic(Object* obj);
ComponentGraphic* Object_GetTerrainGraphic(Object* obj);
ComponentLight* Object_GetLight(Object* obj);
ComponentDefense* Object_GetDefense(Object* obj);
ComponentOffense* Object_GetOffense(Object* obj);

ComponentMonitor* Object_AddMonitor(Object* obj);
ComponentPhysique* Object_AddPhysique(Object* obj);
ComponentGraphic* Object_AddGraphic(Object* obj);
ComponentGraphic* Object_AddTerrainGraphic(Object* obj);
ComponentLight* Object_AddLight(Object* obj);
ComponentDefense* Object_AddDefense(Object* obj);
ComponentOffense* Object_AddOffense(Object* obj);


void Object_Term(Object* obj);

// Different Object types
int ObjectTile_InitFromCfg(Object* obj, const CfgGroundTexture *cfg, Vec2F position);
int ObjectCharacter_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position);
int ObjectCamera_Init(Object* obj);
int ObjectPointer_Init(Object* obj);
int ObjectProjectile_InitFromCfg(Object* obj, const CfgProjectile *cfg, ID originatorId, Vec2F position, Vec2F direction);
int ObjectMelee_InitFromCfg(Object* obj, const CfgMelee *cfg, ID originatorId, Vec2F position, Vec2F direction);
int ObjectExplosive_InitFromCfg(Object* obj, const CfgExplosive *cfg, ID originatorId, Vec2F position, Vec2F direction);
int ObjectWall_Init(Object* obj, Vec2F position);
int ObjectStaticBox_Init(Object* obj, Vec2F position);

#endif
