#ifndef DEBUG_H
#define DEBUG_H

#include "Vec2F.h"
#include "ObjectDrawList.h"
#include <SDL.h>

void DebugVec2F(const char *message, Vec2F vec);
void DebugVec2I(const char *message, Vec2I vec);
void DebugSDLRect(const char *message, SDL_Rect rect);
void DebugKeys(const char *message, uint8_t *keysPressed, uint8_t *keysReleased, uint8_t *keyState);
void DebugObjectDrawList(const char *message, ObjectDrawList *list);

#endif
