#ifndef DEBUG_H
#define DEBUG_H

#include "Vec2F.h"
#include "DrawList.h"
#include <SDL.h>

void DebugVec2F(const char *message, Vec2F vec);
void DebugVec2I(const char *message, Vec2I vec);
void DebugSDLRect(const char *message, SDL_Rect rect);
void DebugKeys(const char *message, uint16_t *keysPressed, uint16_t *keysReleased, uint8_t *keysState);
void DebugButtons(const char *message, uint16_t *buttonsPressed, uint16_t *buttonsReleased, uint8_t *buttonsState);
void DebugDrawList(const char *message, DrawList *list);

#endif
