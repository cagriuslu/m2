#ifndef DIALOG_H
#define DIALOG_H

#include "Array.h"

#define X_WINDOW (1)
#define X_KEY (2)
#define X_MOTION (3)
#define X_BUTTON (4)
#define X_WHEEL (5)
int DialogWaitForEvent(Array *uis);

#define X_MAIN_MENU_RESUME (0)
#define X_MAIN_MENU_NEW_GAME (1)
#define X_MAIN_MENU_LEVEL_EDITOR (2)
int DialogMainMenu();

#endif
