#ifndef OBJECT_H
#define OBJECT_H

#include "EventListenerComponent.h"
#include "PhysicsComponent.h"
#include "GraphicsComponent.h"
#include "Vec2F.h"
#include <stdint.h>

typedef struct _Object {
	Vec2F position; // in world coordinates
	// Components
	uint32_t eventListener;
	uint32_t physics;
	uint32_t graphics;
	uint32_t terrainGraphics;
	// Private Data
	void* privData;
} Object;

int ObjectInit(Object* obj, Vec2F position);
void ObjectDeinit(Object* obj);

EventListenerComponent* ObjectAddAndInitEventListener(Object* obj, uint32_t* outId);
PhysicsComponent* ObjectAddAndInitPhysics(Object* obj, uint32_t* outId);
GraphicsComponent* ObjectAddAndInitGraphics(Object* obj, uint32_t* outId);
GraphicsComponent* ObjectAddAndInitTerrainGraphics(Object* obj, uint32_t* outId);

int ObjectPlayerInit(Object* obj);
int ObjectGodInit(Object* obj);
int ObjectCameraInit(Object* obj);
int ObjectBulletInit(Object* obj, Vec2F position, Vec2F direction);
int ObjectSkeletonInit(Object* obj, Vec2F position);
int ObjectWallInit(Object* obj, Vec2F position);
int ObjectStaticBoxInit(Object* obj, Vec2F position);

#endif
