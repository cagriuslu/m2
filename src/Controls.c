#include "Controls.h"

Key KeyFromSDLScancode(SDL_Scancode sc) {
	switch (sc) {
	case SDL_SCANCODE_W:
		return KEY_UP;
	case SDL_SCANCODE_S:
		return KEY_DOWN;
	case SDL_SCANCODE_A:
		return KEY_LEFT;
	case SDL_SCANCODE_D:
		return KEY_RIGHT;
	default:
		return KEY_NONE;
	}
}

void KeyStateArrayFillFromSDLKeyboardStateArray(uint8_t *keyState, const uint8_t *keyboardState) {
	if (keyboardState[SDL_SCANCODE_W]) {
		keyState[KEY_UP] = 1;
	}
	if (keyboardState[SDL_SCANCODE_S]) {
		keyState[KEY_DOWN] = 1;
	}
	if (keyboardState[SDL_SCANCODE_A]) {
		keyState[KEY_LEFT] = 1;
	}
	if (keyboardState[SDL_SCANCODE_D]) {
		keyState[KEY_RIGHT] = 1;
	}
}
