#ifndef DEBUG_H
#define DEBUG_H

#include "Box2DWrapper.h"
#include "Array.h"
#include "List.h"
#include "Vec2F.h"
#include <SDL.h>

void DebugVec2F(const char *message, Vec2F vec);
void DebugVec2I(const char *message, Vec2I vec);
void DebugSDLRect(const char *message, SDL_Rect rect);
void DebugKeys(const char *message, uint16_t *keysPressed, uint16_t *keysReleased, uint8_t *keysState);
void DebugButtons(const char *message, uint16_t *buttonsPressed, uint16_t *buttonsReleased, uint8_t *buttonsState);
void DebugBox2DAABB(const char* message, Box2DAABB aabb);

void DebugIntArray(const char* message, Array* array);
void DebugVec2IList(const char* message, List* list);

#endif
