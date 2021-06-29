#ifndef OBJECT_H
#define OBJECT_H

#include "Component.h"
#include "TileLookup.h"
#include "AI.h"
#include "Array.h"
#include "Vec2F.h"
#include <stdint.h>

#define FindObjectById(id)               ((Object*) BucketGetById(&CurrentLevel()->objects, (id)))
#define FindObjectOfComponent(component) (FindObjectById((component)->super.objId))

#define FindEventListenerById(id)   ((EventListenerComponent*) BucketGetById(&CurrentLevel()->eventListeners, (id)))
#define FindPhysicsById(id)         ((PhysicsComponent*)       BucketGetById(&CurrentLevel()->physics, (id)))
#define FindGraphicsById(id)        ((GraphicsComponent*)      BucketGetById(&CurrentLevel()->graphics, (id)))
#define FindTerrainGraphicsById(id) ((GraphicsComponent*)      BucketGetById(&CurrentLevel()->terrainGraphics, (id)))
#define FindDefenseById(id)         ((ComponentDefense*)       BucketGetById(&CurrentLevel()->defenses, (id)))
#define FindOffenseById(id)         ((ComponentOffense*)       BucketGetById(&CurrentLevel()->offenses, (id)))

#define FindEventListenerOfObject(obj)   (FindEventListenerById((obj->eventListener)))
#define FindPhysicsOfObject(obj)         (FindPhysicsById((obj->physics)))
#define FindGraphicsOfObject(obj)        (FindGraphicsById((obj->graphics)))
#define FindTerrainGraphicsOfObject(obj) (FindTerrainGraphicsById((obj->terrainGraphics)))
#define FindDefenseOfObject(obj)         (FindDefenseById((obj->defense)))
#define FindOffenseOfObject(obj)         (FindOffenseById((obj->offense)))

#define DeleteObjectById(id) do { uint64_t __id__ = (id); ArrayAppend(&CurrentLevel()->deleteList, &__id__); } while (0)
#define DeleteObject(obj)    DeleteObjectById(BucketGetId(&CurrentLevel()->objects, (obj)))

typedef struct _Object {
	Vec2F position; // in world coordinates
	// Components
	uint64_t eventListener;
	uint64_t physics;
	uint64_t graphics;
	uint64_t terrainGraphics;
	uint64_t defense;
	uint64_t offense;
	uint64_t lightSource;
	// Properties
	AI* ai;
	uint64_t prePhysicsStopwatches;
} Object;

int ObjectInit(Object* obj, Vec2F position);
void ObjectDeinit(Object* obj);

EventListenerComponent* ObjectAddEventListener(Object* obj, uint64_t* outId);
PhysicsComponent* ObjectAddPhysics(Object* obj, uint64_t* outId);
GraphicsComponent* ObjectAddGraphics(Object* obj, uint64_t* outId);
GraphicsComponent* ObjectAddTerrainGraphics(Object* obj, uint64_t* outId);
ComponentDefense* ObjectAddDefense(Object* obj, uint64_t* outId);
ComponentOffense* ObjectAddOffense(Object* obj, uint64_t* outId);
ComponentLightSource* ObjectAddLightSource(Object* obj, float lightBoundaryRadius, uint64_t* outId);

Array* ObjectAddPrePhysicsStopwatches(Object* obj, unsigned stopwatchCount);

int ObjectTileInit(Object* obj, TileDef tileDef, Vec2F position);
int ObjectPlayerInit(Object* obj);
int ObjectGodInit(Object* obj);
int ObjectCameraInit(Object* obj);
int ObjectBulletInit(Object* obj, Vec2F position, Vec2F direction, ComponentOffense** outOffense);
int ObjectSkeletonInit(Object* obj, Vec2F position);
int ObjectWallInit(Object* obj, Vec2F position);
int ObjectStaticBoxInit(Object* obj, Vec2F position);
int ObjectSwordInit(Object* obj, Vec2F originatorPosition, ComponentOffense* originatorOffense, Vec2F direction, uint32_t ticks);

#endif
