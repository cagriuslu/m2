#ifndef DIALOG_H
#define DIALOG_H

#include "Game.h"
#include "Array.h"
#include <stdbool.h>

#define X_WINDOW (1)
#define X_KEY (2)
#define X_MOTION (3)
#define X_BUTTON (4)
#define X_WHEEL (5)
int DialogWaitForEvent(Game *game, Array *uis);

#define X_MAIN_MENU_RESUME (1)
#define X_MAIN_MENU_NEW_GAME (2)
#define X_MAIN_MENU_LEVEL_EDITOR (3)
int DialogMainMenu(Game *game, bool levelLoaded);

#endif
