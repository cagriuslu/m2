#ifndef UI_H
#define UI_H

#include "Game.h"
#include "Vec2F.h"
#include "Vec2I.h"
#include <SDL.h>
#include <stdbool.h>

typedef struct _Ui {
	SDL_Rect trigZone;
	void *eventData;
	void (*onMouseButton)(struct _Ui*);
	void (*draw)(struct _Ui*, Game *game);
	void *privData;
} Ui;

int Ui_Init(Ui *ui);
void Ui_Term(Ui *ui);

int UiButton_Init(Ui *ui, Vec2I position, Vec2I minSize, Vec2I padding, int alignment, const char *text);
Vec2I UiButton_Size(Ui *ui);
void UiButton_SetSize(Ui *ui, Vec2I size);
void UiButton_Term(Ui *ui);
Vec2I UiButton_MaxSizeOfButtons(unsigned n, ...);

#endif
