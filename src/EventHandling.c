#include "EventHandling.h"

uint16_t gKeysPressed[_KEY_COUNT];
uint16_t gKeysReleased[_KEY_COUNT];
uint8_t gKeysState[_KEY_COUNT];

uint16_t gButtonsPressed[_BUTTON_COUNT];
uint16_t gButtonsReleased[_BUTTON_COUNT];
uint8_t gButtonsState[_BUTTON_COUNT];
Vec2I gPointerPosition;

void GatherEvents(bool *outQuit, bool *outWindow, bool *outKey, bool *outMotion, bool *outButton, bool *outWheel) {
	if (outQuit) {
		*outQuit = false;
	}
	if (outWindow) {
		*outWindow = false;
	}
	if (outKey) {
		*outKey = false;
	}
	if (outMotion) {
		*outMotion = false;
	}
	if (outButton) {
		*outButton = false;
	}
	if (outWheel) {
		*outWheel = false;
	}
	// Clear events
	memset(gKeysPressed, 0, sizeof(gKeysPressed));
	memset(gKeysReleased, 0, sizeof(gKeysReleased));
	memset(gKeysState, 0, sizeof(gKeysState));
	memset(gButtonsPressed, 0, sizeof(gButtonsPressed));
	memset(gButtonsReleased, 0, sizeof(gButtonsReleased));
	memset(gButtonsState, 0, sizeof(gButtonsState));
	// Handle events
	SDL_Event e;
	while ((*outQuit == false) && SDL_PollEvent(&e) != 0) {
		switch (e.type) {
		case SDL_QUIT:
			if (outQuit) {
				*outQuit = true;
			}
			break;
		case SDL_WINDOWEVENT:
			if (outWindow) {
				*outWindow = true;
			}
			break;
		case SDL_KEYDOWN:
			if (e.key.repeat == 0) {
				if (outKey) {
					*outKey = true;
				}
				gKeysPressed[KeyFromSDLScancode(e.key.keysym.scancode)] += 1;
			}
			break;
		case SDL_KEYUP:
			if (e.key.repeat == 0) {
				if (outKey) {
					*outKey = true;
				}
				gKeysReleased[KeyFromSDLScancode(e.key.keysym.scancode)] += 1;
			}
			break;
		case SDL_MOUSEMOTION:
			if (outMotion) {
				*outMotion = true;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (e.button.clicks == 1) {
				if (outButton) {
					*outButton = true;
				}
				gButtonsPressed[ButtonFromSDLButton(e.button.button)] += 1;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			if (e.button.clicks == 1) {
				if (outButton) {
					*outButton = true;
				}
				gButtonsReleased[ButtonFromSDLButton(e.button.button)] += 1;
			}
			break;
		case SDL_MOUSEWHEEL:
			if (outWheel) {
				*outWheel = true;
			}
			break;
		default:
			break;
		}
	}
	KeyStateArrayFillFromSDLKeyboardStateArray(gKeysState, SDL_GetKeyboardState(NULL));
	ButtonStateArrayFillFromSDLMouseState(gButtonsState, SDL_GetMouseState(&gPointerPosition.x, &gPointerPosition.y));
}

uint16_t IsKeyPressed(Key key) {
	return gKeysPressed[key];
}

uint16_t IsKeyReleased(Key key) {
	return gKeysReleased[key];
}

bool IsKeyDown(Key key) {
	return gKeysState[key];
}

uint16_t IsButtonPressed(Button button) {
	return gButtonsPressed[button];
}

uint16_t IsButtonReleased(Button button) {
	return gButtonsReleased[button];
}

bool IsButtonDown(Button button) {
	return gButtonsState[button];
}

Vec2I PointerPosition() {
	return gPointerPosition;
}

uint16_t* KeysPressedArray() {
	return gKeysPressed;
}

uint16_t* KeysReleasedArray() {
	return gKeysReleased;
}

uint8_t* KeysStateArray() {
	return gKeysState;
}

uint16_t* ButtonsPressedArray() {
	return gButtonsPressed;
}

uint16_t* ButtonsReleasedArray() {
	return gButtonsReleased;
}

uint8_t* ButtonsStateArray() {
	return gButtonsState;
}
