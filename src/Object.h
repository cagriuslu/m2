#ifndef OBJECT_H
#define OBJECT_H

#include "Point.h"
#include "Geometry.h"
#include "Error.h"
#include <SDL.h>

typedef struct _Object {
	Point pos; // Position of the origin of the object
	// Physics subsystem
	Geometry geo; // Geometry
	Point vel; // Velocity
	Point avel; // Angular velocity
	Point drag; // Drag
	Point aDrag; // Angular drag
	float mass; // Mass
	Point grav; // Gravitational force
	void (*prePhysics)(struct _Object*);
	void (*onCollision)(struct _Object*);
	void (*postPhysics)(struct _Object*);
	// Trigger subsystem
	Geometry trig; // Geometry
	void (*onTrigger)(struct _Object*);
	// Graphics subsystem
	SDL_Texture *tx;
	SDL_Rect txSrc;
	Point txOff; // Texture offset
	float txScaleW, txScaleH;
	float txRotZ;
	void (*preGraphics)(struct _Object*);
	void (*postGraphics)(struct _Object*);
	// Private data
	void *privData;
} Object;

int ObjectInit(Object *obj);

#endif
