#include "Event.h"
#include <string.h>

int gNewScreenWidth;
int gNewScreenHeight;

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
	gNewScreenWidth = 0;
	gNewScreenHeight = 0;
	memset(gKeysPressed, 0, sizeof(gKeysPressed));
	memset(gKeysReleased, 0, sizeof(gKeysReleased));
	memset(gKeysState, 0, sizeof(gKeysState));
	memset(gButtonsPressed, 0, sizeof(gButtonsPressed));
	memset(gButtonsReleased, 0, sizeof(gButtonsReleased));
	memset(gButtonsState, 0, sizeof(gButtonsState));
	// Handle events
	bool quit = false;
	SDL_Event e;
	while (quit == false && SDL_PollEvent(&e) != 0) {
		switch (e.type) {
		case SDL_QUIT:
			quit = true;
			if (outQuit) {
				*outQuit = true;
			}
			break;
		case SDL_WINDOWEVENT:
			if (outWindow) {
				*outWindow = true;
			}
			switch (e.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					gNewScreenWidth = e.window.data1;
					gNewScreenHeight = e.window.data2;
					break;
			}
			break;
		case SDL_KEYDOWN:
			if (e.key.repeat == 0) {
				if (outKey) {
					*outKey = true;
				}
				gKeysPressed[KeyFromSDLScancode(e.key.keysym.scancode, e.key.keysym.mod)] += 1;
			}
			break;
		case SDL_KEYUP:
			if (e.key.repeat == 0) {
				if (outKey) {
					*outKey = true;
				}
				gKeysReleased[KeyFromSDLScancode(e.key.keysym.scancode, e.key.keysym.mod)] += 1;
			}
			break;
		case SDL_MOUSEMOTION:
			if (outMotion) {
				*outMotion = true;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (outButton) {
				*outButton = true;
			}
			gButtonsPressed[ButtonFromSDLButton(e.button.button)] += 1;
			break;
		case SDL_MOUSEBUTTONUP:
			if (outButton) {
				*outButton = true;
			}
			gButtonsReleased[ButtonFromSDLButton(e.button.button)] += 1;
			break;
		case SDL_MOUSEWHEEL:
			if (outWheel) {
				*outWheel = true;
			}
			if (0 < e.wheel.y) {
				gButtonsPressed[BUTTON_SCROLL_UP] += e.wheel.y;
			} else {
				gButtonsPressed[BUTTON_SCROLL_DOWN] += -e.wheel.y;
			}
			break;
		default:
			break;
		}
	}
	KeyStateArrayFillFromSDLKeyboardStateArray(gKeysState, SDL_GetKeyboardState(NULL));
	ButtonStateArrayFillFromSDLMouseState(gButtonsState, SDL_GetMouseState(&gPointerPosition.x, &gPointerPosition.y));
}

Vec2I IsScreenResized() {
	return (Vec2I) { gNewScreenWidth, gNewScreenHeight };
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

uint16_t IsButtonPressed(MouseButton button) {
	return gButtonsPressed[button];
}

uint16_t IsButtonReleased(MouseButton button) {
	return gButtonsReleased[button];
}

bool IsButtonDown(MouseButton button) {
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
