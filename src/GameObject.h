#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Box2DWrapper.h"
#include "Vec2F.h"
#include "Object.h"
#include "Error.h"
#include <SDL.h>

typedef struct _GameObject {
	Object super;
	Vec2F pos;
	float angle; // Radians
	// Physics subsystem
	Box2DBody *body;
	void (*prePhysics)(struct _GameObject*);
	void (*onCollision)(struct _GameObject*, Object*);
	void (*postPhysics)(struct _GameObject*);
	// Graphics subsystem
	SDL_Texture* tx;
	SDL_Rect txSrc;
	Vec2F txOffset; // w.r.t source image, # of pixels
	void (*preGraphics)(struct _GameObject*);
	void (*ovrdGraphics)(struct _GameObject*);
	void (*postGraphics)(struct _GameObject*);
	// Private data
	void *privData;
	// Deinit
	void (*deinit)(struct _GameObject*);
} GameObject;
typedef GameObject* ObjectPtr;

int ObjectInit(GameObject *obj);
void ObjectDeinit(GameObject *obj);

void ObjectContactCB(Box2DContact* contact);

#endif
