#ifndef UI_H
#define UI_H

#include "Vec2F.h"
#include "Vec2I.h"
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

int UiButtonInit(Ui *ui, Vec2I position, Vec2I minSize, Vec2I padding, int alignment, const char *text);
Vec2I UiButtonSize(Ui *ui);
void UiButtonSetSize(Ui *ui, Vec2I size);
void UiButtonDeinit(Ui *ui);
Vec2I UiButtonMaxSizeOfButtons(unsigned n, ...);

#endif
