#ifndef HUD_WIDGET_H
#define HUD_WIDGET_H

#include <SDL.h>

typedef struct _HudWidget {
	void* hudBackPtr;
	void (*draw)(struct _HudWidget, SDL_Rect);
} HudWidget;

#endif
