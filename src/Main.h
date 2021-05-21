#ifndef MAIN_H
#define MAIN_H

#include "Controls.h"
#include <SDL.h>
#include <stdbool.h>

SDL_Renderer* CurrentRenderer();
bool IsKeyPressed(Key key);
bool IsKeyReleased(Key key);
bool IsKeyDown(Key key);

#endif
