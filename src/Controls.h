#ifndef CONTROLS_H
#define CONTROLS_H

#include <SDL.h>

typedef enum _Key {
	KEY_NONE = 0,

	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,

	_KEY_COUNT
} Key;

Key KeyFromSDLScancode(SDL_Scancode sc);
void KeyStateArrayFillFromSDLKeyboardStateArray(uint8_t *keyState, const uint8_t *keyboardState);

#endif
