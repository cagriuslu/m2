#ifndef OBJECT_H
#define OBJECT_H

#include "Vec2F.h"
#include "Geometry.h"
#include "Error.h"
#include <SDL.h>

typedef struct _Object {
	Vec2F pos; // Position of the origin of the object
	// Physics subsystem
	Geometry geo; // Geometry
	Vec2F vel; // Velocity
	float avel; // Angular velocity
	float drag; // Drag
	float aDrag; // Angular drag
	float mass; // Mass
	float grav; // Gravitational force
	void (*prePhysics)(struct _Object*);
	void (*onCollision)(struct _Object*);
	void (*postPhysics)(struct _Object*);
	// Trigger subsystem
	Geometry trig; // Geometry
	void (*onTrigger)(struct _Object*);
	// Graphics subsystem
	SDL_Texture *tx;
	SDL_Rect txSrc;
	Vec2I txOff; // Texture offset wrt object origin
	float txScaleW, txScaleH;
	float txRotZ; // Ignored for now
	void (*preGraphics)(struct _Object*);
	void (*ovrdGraphics)(struct _Object*, SDL_Renderer *renderer);
	void (*postGraphics)(struct _Object*);
	// Private data
	void *privData;
} Object;

int ObjectInit(Object *obj);
void ObjectDeinit(Object *obj);

#endif
