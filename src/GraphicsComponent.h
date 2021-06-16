#ifndef GRAPHICS_COMPONENT_H
#define GRAPHICS_COMPONENT_H

#include "Component.h"
#include <SDL.h>
#include "Vec2F.h"

typedef struct _GraphicsComponent {
	Component super;
	SDL_Texture* tx;
	SDL_Rect txSrc;
	float txAngle;
	Vec2F txOffset; // w.r.t tx, # of pixels
	void (*draw)(struct _GraphicsComponent*);
} GraphicsComponent;

int GraphicsComponentInit(GraphicsComponent* gfx, uint32_t objectId);
void GraphicsComponentDeinit(GraphicsComponent* gfx);

int GraphicsComponentYComparatorCB(void* gfxIdAPtr, void* gfxIdBPtr);

#endif
