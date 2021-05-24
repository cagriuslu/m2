#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "Ui.h"
#include "../Vec2I.h"

int UiButtonInit(Ui *ui, Vec2I position, Vec2I minSize, Vec2I padding, int alignment, const char *text);
Vec2I UIButtonSize(Ui *ui);
void UIButtonSetSize(Ui *ui, Vec2I size);
void UiButtonDeinit(Ui *ui);

#endif
