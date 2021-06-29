#ifndef OBJECT_H
#define OBJECT_H

#include "Component.h"
#include "TileLookup.h"
#include "AI.h"
#include "Array.h"
#include "Vec2F.h"
#include <stdint.h>

#define FindObjectById(id)               ((Object*) Bucket_GetById(&CurrentLevel()->objects, (id)))
#define FindObjectOfComponent(component) (FindObjectById((component)->super.objId))

#define FindEventListenerById(id)   ((EventListenerComponent*) Bucket_GetById(&CurrentLevel()->eventListeners, (id)))
#define FindPhysicsById(id)         ((PhysicsComponent*)       Bucket_GetById(&CurrentLevel()->physics, (id)))
#define FindGraphicsById(id)        ((GraphicsComponent*)      Bucket_GetById(&CurrentLevel()->graphics, (id)))
#define FindTerrainGraphicsById(id) ((GraphicsComponent*)      Bucket_GetById(&CurrentLevel()->terrainGraphics, (id)))
#define FindDefenseById(id)         ((ComponentDefense*)       Bucket_GetById(&CurrentLevel()->defenses, (id)))
#define FindOffenseById(id)         ((ComponentOffense*)       Bucket_GetById(&CurrentLevel()->offenses, (id)))

#define FindEventListenerOfObject(obj)   (FindEventListenerById((obj->eventListener)))
#define FindPhysicsOfObject(obj)         (FindPhysicsById((obj->physics)))
#define FindGraphicsOfObject(obj)        (FindGraphicsById((obj->graphics)))
#define FindTerrainGraphicsOfObject(obj) (FindTerrainGraphicsById((obj->terrainGraphics)))
#define FindDefenseOfObject(obj)         (FindDefenseById((obj->defense)))
#define FindOffenseOfObject(obj)         (FindOffenseById((obj->offense)))

#define DeleteObjectById(id) do { ID __id__ = (id); Array_Append(&CurrentLevel()->deleteList, &__id__); } while (0)
#define DeleteObject(obj)    DeleteObjectById(Bucket_GetId(&CurrentLevel()->objects, (obj)))

typedef struct _Object {
	Vec2F position; // in world coordinates
	// Components
	ID eventListener;
	ID physics;
	ID graphics;
	ID terrainGraphics;
	ID defense;
	ID offense;
	ID lightSource;
	// Properties
	AI* ai;
	ID prePhysicsStopwatches;
} Object;

int ObjectInit(Object* obj, Vec2F position);
void ObjectDeinit(Object* obj);

EventListenerComponent* ObjectAddEventListener(Object* obj, ID* outId);
PhysicsComponent* ObjectAddPhysics(Object* obj, ID* outId);
GraphicsComponent* ObjectAddGraphics(Object* obj, ID* outId);
GraphicsComponent* ObjectAddTerrainGraphics(Object* obj, ID* outId);
ComponentDefense* ObjectAddDefense(Object* obj, ID* outId);
ComponentOffense* ObjectAddOffense(Object* obj, ID* outId);
ComponentLightSource* ObjectAddLightSource(Object* obj, float lightBoundaryRadius, ID* outId);

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
