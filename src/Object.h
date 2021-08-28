#ifndef OBJECT_H
#define OBJECT_H

#include "Component.h"
#include "AI.h"
#include "Array.h"
#include "Item.h"
#include "Character.h"
#include "Vec2F.h"
#include <stdint.h>

#define FindObjectById(id)               ((Object*) Bucket_GetById(&CurrentLevel()->objects, (id)))
#define FindObjectOfComponent(component) (FindObjectById((component)->super.objId))

#define FindEventListenerById(id)     ((ComponentEventListener*) Bucket_GetById(&CurrentLevel()->eventListeners, (id)))
#define FindPhysicsById(id)           ((ComponentPhysics*)       Bucket_GetById(&CurrentLevel()->physics, (id)))
#define FindGraphicsById(id)          ((ComponentGraphics*)      Bucket_GetById(&CurrentLevel()->graphics, (id)))
#define FindTerrainGraphicsById(id)   ((ComponentGraphics*)      Bucket_GetById(&CurrentLevel()->terrainGraphics, (id)))
#define FindDefenseById(id)           ((ComponentDefense*)       Bucket_GetById(&CurrentLevel()->defenses, (id)))
#define FindOffenseProjectileById(id) ((ComponentOffense*)       Bucket_GetById(&CurrentLevel()->offenses, (id)))
#define FindOffenseMeleeById(id)      ((ComponentOffense*)       Bucket_GetById(&CurrentLevel()->offenses, (id)))

#define FindEventListenerOfObject(obj)     (FindEventListenerById((obj->eventListener)))
#define FindPhysicsOfObject(obj)           (FindPhysicsById((obj->physics)))
#define FindGraphicsOfObject(obj)          (FindGraphicsById((obj->graphics)))
#define FindTerrainGraphicsOfObject(obj)   (FindTerrainGraphicsById((obj->terrainGraphics)))
#define FindDefenseOfObject(obj)           (FindDefenseById((obj->defense)))
#define FindOffenseProjectileOfObject(obj) (FindOffenseProjectileById((obj->offenseProjectile)))
#define FindOffenseMeleeOfObject(obj)      (FindOffenseMeleeById((obj->offenseMelee)))

#define DeleteObjectById(id) do { ID __id__ = (id); Array_Append(&CurrentLevel()->deleteList, &__id__); } while (0)
#define DeleteObject(obj)    DeleteObjectById(Bucket_GetId(&CurrentLevel()->objects, (obj)))

typedef struct _ObjectProperties {
	// Used by Player
	Character* character;
	Stopwatch rangedAttackStopwatch;
	Stopwatch meleeAttackStopwatch;
	// Used by Enemy
	AI* ai;
} ObjectProperties;

int ObjectProperties_Init(ObjectProperties* props);
void ObjectProperties_Term(ObjectProperties* props);

/// Basis of all objects in the game.
/// 
/// How to decide if a component should reside in Bucket or be held as a property?
/// If the component is accessed by the Main Game Loop => Bucket
/// If the component is created and destroyed rapidly => Bucket
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
	ID offenseProjectile;
	ID offenseMelee;
	ID lightSource;
	ObjectProperties* properties;
} Object;

int Object_Init(Object* obj, Vec2F position, bool initProperties);
void Object_Term(Object* obj);

ComponentEventListener* Object_AddEventListener(Object* obj, ID* outId);
ComponentPhysics* Object_AddPhysics(Object* obj, ID* outId);
ComponentGraphics* Object_AddGraphics(Object* obj, ID* outId);
ComponentGraphics* Object_AddTerrainGraphics(Object* obj, ID* outId);
ComponentDefense* Object_AddDefense(Object* obj, ID* outId);
ComponentOffense* Object_AddOffenseProjectile(Object* obj, ID* outId);
ComponentOffense* Object_AddOffenseMelee(Object* obj, ID* outId);
ComponentLightSource* Object_AddLightSource(Object* obj, float lightBoundaryRadius, ID* outId);

typedef struct _TileDef {
	Vec2I txIndex;
	Vec2F colliderSize;
	Vec2F colliderOffset;
} TileDef;

int ObjectTile_Init(Object* obj, TileDef tileDef, Vec2F position);
int ObjectPlayer_Init(Object* obj, Character* character);
int ObjectGod_Init(Object* obj);
int ObjectCamera_Init(Object* obj);
int ObjectBullet_Init(Object* obj, Vec2F position, Vec2F direction, ItemType projectileType, ComponentOffense* copyOffense);
int ObjectEnemy_Init(Object* obj, Vec2F position, const char* descriptor);
int ObjectWall_Init(Object* obj, Vec2F position);
int ObjectStaticBox_Init(Object* obj, Vec2F position);
int ObjectSword_Init(Object* obj, Vec2F originatorPosition, ComponentOffense* originatorOffense, bool isEnemy, Vec2F direction, uint32_t ticks);

#endif
