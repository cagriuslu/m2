#ifndef UI_H
#define UI_H

#include <SDL.h>
#include "Vec2F.h"

typedef struct _Ui {
	SDL_Rect trigZone;
	void (*onMouseEnter)(struct _Ui*);
	void (*onMouseButton)(struct _Ui*);
	void (*onMouseExit)(struct _Ui*);
	void (*draw)(struct _Ui*);
	void *privData;
} Ui;

int UiInit(Ui *ui);
void UiDeinit(Ui *ui);

#endif
