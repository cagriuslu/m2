#ifndef OBJECT_H
#define OBJECT_H

#include "Component.h"
#include "TileLookup.h"
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

#define DeleteObjectById(id) do { uint32_t __id__ = (id); ArrayAppend(&CurrentLevel()->deleteList, &__id__); } while (0)
#define DeleteObject(obj)    DeleteObjectById(BucketGetId(&CurrentLevel()->objects, (obj)))

typedef struct _Object {
	Vec2F position; // in world coordinates
	// Components
	uint32_t eventListener;
	uint32_t physics;
	uint32_t graphics;
	uint32_t terrainGraphics;
	uint32_t defense;
	uint32_t offense;
} Object;

int ObjectInit(Object* obj, Vec2F position);
void ObjectDeinit(Object* obj);

EventListenerComponent* ObjectAddAndInitEventListener(Object* obj, uint32_t* outId);
PhysicsComponent* ObjectAddAndInitPhysics(Object* obj, uint32_t* outId);
GraphicsComponent* ObjectAddAndInitGraphics(Object* obj, uint32_t* outId);
GraphicsComponent* ObjectAddAndInitTerrainGraphics(Object* obj, uint32_t* outId);
ComponentDefense* ObjectAddAndInitDefense(Object* obj, uint32_t* outId);
ComponentOffense* ObjectAddAndInitOffense(Object* obj, uint32_t* outId);

int ObjectTileInit(Object* obj, TileDef tileDef, Vec2F position);
int ObjectPlayerInit(Object* obj);
int ObjectGodInit(Object* obj);
int ObjectCameraInit(Object* obj);
int ObjectBulletInit(Object* obj, Vec2F position, Vec2F direction, ComponentOffense** outOffense);
int ObjectSkeletonInit(Object* obj, Vec2F position);
int ObjectWallInit(Object* obj, Vec2F position);
int ObjectStaticBoxInit(Object* obj, Vec2F position);

#endif
