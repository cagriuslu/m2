#ifndef UI_H
#define UI_H

#include "../Vec2F.h"
#include <SDL.h>
#include <stdbool.h>

typedef struct _Ui {
	SDL_Rect trigZone;
	void *eventData;
	void (*onMouseButton)(struct _Ui*);
	void (*draw)(struct _Ui*);
	void *privData;
} Ui;

int UiInit(Ui *ui);
void UiDeinit(Ui *ui);

#endif
