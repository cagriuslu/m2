#ifndef HUD_COMPLICATION_H
#define HUD_COMPLICATION_H

#include <SDL.h>

typedef struct _HudComplication {
	void* hudBackPtr;
	void (*draw)(struct _HudComplication, SDL_Rect);
} HudComplication;

#endif
