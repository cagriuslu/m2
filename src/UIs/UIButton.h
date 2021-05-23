#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "../UI.h"
#include "../Vec2I.h"

int UIButtonInit(UI *ui, Vec2I size, int alignment, const char *text);
void UIButtonDeinit(UI *ui);

#endif
