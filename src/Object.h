#ifndef OBJECT_H
#define OBJECT_H

#include "Component.h"
#include "TileLookup.h"
#include "Vec2F.h"
#include <stdint.h>

#define FindObjectById(id)               ((Object*) BucketGetById(&CurrentLevel()->objects, (id)))
#define FindObjectOfComponent(component) (FindObjectById((component)->super.objId))

#define FindDefenseById(id)      ((ComponentDefense*) BucketGetById(&CurrentLevel()->defenses, (id)))
#define FindDefenseOfObject(obj) (FindDefenseById((obj->defense)))

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
