#ifndef MAIN_H
#define MAIN_H

#include "Controls.h"
#include "Box2DWrapper.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

int CurrentScreenWidth();
int CurrentScreenHeight();
SDL_Renderer* CurrentRenderer();
SDL_Texture* CurrentTextureLUT();
TTF_Font* CurrentFont();
Box2DWorld* CurrentWorld();
bool IsKeyPressed(Key key);
bool IsKeyReleased(Key key);
bool IsKeyDown(Key key);

#endif
