#include "Controls.h"

Key KeyFromSDLScancode(SDL_Scancode sc, SDL_Keymod mod) {
	switch (sc) {
	case SDL_SCANCODE_ESCAPE:
		return KEY_MENU;
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
	// If any modifier key is pressed, skip altogether
	if (keyboardState[SDL_SCANCODE_LCTRL] || keyboardState[SDL_SCANCODE_RCTRL] ||
		keyboardState[SDL_SCANCODE_LALT] || keyboardState[SDL_SCANCODE_RALT]) {
		return;
	}

	if (keyboardState[SDL_SCANCODE_ESCAPE]) {
		keyState[KEY_MENU] = 1;
	}
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

MouseButton ButtonFromSDLButton(int button) {
	switch (button) {
	case SDL_BUTTON_LEFT:
		return BUTTON_PRIMARY;
	case SDL_BUTTON_RIGHT:
		return BUTTON_SECONDARY;
	default:
		return BUTTON_NONE;
	}
}

void ButtonStateArrayFillFromSDLMouseState(uint8_t *buttonState, const uint32_t bitmask) {
	if (bitmask & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		buttonState[BUTTON_PRIMARY] = 1;
	}
	if (bitmask & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
		buttonState[BUTTON_SECONDARY] = 1;
	}
}
