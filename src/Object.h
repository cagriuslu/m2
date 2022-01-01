#ifndef OBJECT_H
#define OBJECT_H

#include "Component.h"
#include "AI.h"
#include "Array.h"
#include "Cfg.h"
#include "Vec2F.h"
#include <stdint.h>

#define FindObjectById(id)               ((Object*) Pool_GetById(&GAME->objects, (id)))
#define FindObjectOfComponent(component) (FindObjectById((component)->super.objId))

#define FindEventListenerById(id)     ((ComponentEventListener*) Pool_GetById(&GAME->eventListeners, (id)))
#define FindPhysicsById(id)           ((ComponentPhysics*)       Pool_GetById(&GAME->physics, (id)))
#define FindGraphicsById(id)          ((ComponentGraphics*)      Pool_GetById(&GAME->graphics, (id)))
#define FindTerrainGraphicsById(id)   ((ComponentGraphics*)      Pool_GetById(&GAME->terrainGraphics, (id)))
#define FindDefenseById(id)           ((ComponentDefense*)       Pool_GetById(&GAME->defenses, (id)))
#define FindOffenseById(id)           ((ComponentOffense*)       Pool_GetById(&GAME->offenses, (id)))

#define FindEventListenerOfObject(obj)     (FindEventListenerById((obj->eventListener)))
#define FindPhysicsOfObject(obj)           (FindPhysicsById((obj->physics)))
#define FindGraphicsOfObject(obj)          (FindGraphicsById((obj->graphics)))
#define FindTerrainGraphicsOfObject(obj)   (FindTerrainGraphicsById((obj->terrainGraphics)))
#define FindDefenseOfObject(obj)           (FindDefenseById((obj->defense)))
#define FindOffenseOfObject(obj)           (FindOffenseById((obj->offense)))

#define DeleteObjectById(id) do { ID __id__ = (id); Array_Append(&GAME->deleteList, &__id__); } while (0)
#define DeleteObject(obj)    DeleteObjectById(Pool_GetId(&GAME->objects, (obj)))

typedef struct _ObjectEx {
	CfgObjectType type;
	union {
		struct {
			CharacterState characterState;
			const CfgCharacter* chr;
			Stopwatch rangedAttackStopwatch;
			Stopwatch meleeAttackStopwatch;
		} player;
		struct {
			AI* ai;
		} enemy;
	} value;
} ObjectEx;
/// Basis of all objects in the game.
/// 
/// How to decide if a component should reside in Pool or be held as a property?
/// If the component is accessed by the Main Game Loop => Pool
/// If the component is created and destroyed rapidly => Pool
/// Others => Property
/// 
typedef struct _Object {
	Vec2F position; // in world coordinates
	// Components
	ID eventListener;
	ID physics;
	ID graphics;
	ID terrainGraphics;
	ID defense;
	ID offense;
	ObjectEx* ex;
} Object;

int Object_Init(Object* obj, Vec2F position, bool initProperties);
void Object_Term(Object* obj);

ComponentEventListener* Object_AddEventListener(Object* obj);
ComponentPhysics* Object_AddPhysics(Object* obj);
ComponentGraphics* Object_AddGraphics(Object* obj);
ComponentGraphics* Object_AddTerrainGraphics(Object* obj);
ComponentDefense* Object_AddDefense(Object* obj);
ComponentOffense* Object_AddOffense(Object* obj);

int ObjectTile_InitFromCfg(Object* obj, const CfgGroundTexture *cfg, Vec2F position);
int ObjectCharacter_InitFromCfg(Object* obj, const CfgCharacter *cfg, Vec2F position);
int ObjectCamera_Init(Object* obj);
int ObjectProjectile_InitFromCfg(Object* obj, const CfgProjectile *cfg, ID originatorId, Vec2F position, Vec2F direction);
int ObjectMelee_InitFromCfg(Object* obj, const CfgMelee *cfg, ID originatorId, Vec2F position, Vec2F direction);
int ObjectExplosive_InitFromCfg(Object* obj, const CfgExplosive *cfg, ID originatorId, Vec2F position, Vec2F direction);

int ObjectWall_Init(Object* obj, Vec2F position);
int ObjectStaticBox_Init(Object* obj, Vec2F position);

#endif
