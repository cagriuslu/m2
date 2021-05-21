#ifndef OBJECT_H
#define OBJECT_H

#include "Vec2F.h"
#include "Geometry.h"
#include "Error.h"
#include <SDL.h>

typedef struct _Object {
	Vec2F pos; // Position of the origin of the object
	// Physics subsystem
	Geometry geo;
	float drag;
	float angDrag;
	void (*prePhysics)(struct _Object*);
	void (*onCollision)(struct _Object*);
	void (*postPhysics)(struct _Object*);
	// Trigger subsystem
	Geometry trig;
	void (*onTrigger)(struct _Object*);
	// Graphics subsystem
	SDL_Texture *tx;
	SDL_Rect txSrc;
	Vec2I txOff;
	float txScaleW, txScaleH;
	float txRotZ;
	void (*preGraphics)(struct _Object*);
	void (*ovrdGraphics)(struct _Object*);
	void (*postGraphics)(struct _Object*);
	// Private data
	void *privData;
} Object;

int ObjectInit(Object *obj);
void ObjectDeinit(Object *obj);

#endif
