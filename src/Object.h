#ifndef OBJECT_H
#define OBJECT_H

#include "Box2DWrapper.h"
#include "Vec2F.h"
#include "Error.h"
#include <SDL.h>

typedef struct _Object {
	Vec2F pos;
	float angle;
	// Physics subsystem
	Box2DBody *body;
	void (*prePhysics)(struct _Object*);
	void (*onCollision)(struct _Object*);
	void (*onTrigger)(struct _Object*);
	void (*postPhysics)(struct _Object*);
	// Graphics subsystem
	SDL_Rect txSrc;
	Vec2I txOffset;
	Vec2F txSize;
	void (*preGraphics)(struct _Object*);
	void (*ovrdGraphics)(struct _Object*);
	void (*postGraphics)(struct _Object*);
	// Private data
	void *privData;
} Object;
typedef Object* ObjectPtr;

int ObjectInit(Object *obj);
void ObjectDeinit(Object *obj);

#endif
