#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include "../Ui.h"
#include "../Vec2I.h"

int UiButtonInit(Ui *ui, Vec2I size, int alignment, const char *text);
void UiButtonDeinit(Ui *ui);

#endif
