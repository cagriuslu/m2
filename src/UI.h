#ifndef UI_H
#define UI_H

#include <SDL.h>
#include "Vec2F.h"

typedef struct _UI {
	SDL_Rect trigZone;
	void (*onMouseEnter)(struct _UI*);
	void (*onMouseButton)(struct _UI*);
	void (*onMouseExit)(struct _UI*);
	void (*draw)(struct _UI*);
	void *privData;
} UI;

int UIInit(UI *ui);
void UIDeinit(UI *ui);

#endif
