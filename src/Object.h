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
	SDL_Texture* tx;
	SDL_Rect txSrc;
	Vec2F txOffset; // w.r.t source image, # of pixels
	void (*preGraphics)(struct _Object*);
	void (*ovrdGraphics)(struct _Object*);
	void (*postGraphics)(struct _Object*);
	// Private data
	void *privData;
	// Deinit
	void (*deinit)(struct _Object*);
} Object;
typedef Object* ObjectPtr;

int ObjectInit(Object *obj);
void ObjectDeinit(Object *obj);

void ObjectContactCB(Box2DContact* contact);

#endif
