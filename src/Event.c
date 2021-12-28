#define _CRT_SECURE_NO_WARNINGS
#include "Event.h"
#include "Log.h"
#include <string.h>
#include <stdio.h>

bool Events_Gather(Events* evs) {
	memset(evs, 0, sizeof(Events));

	SDL_Event e;
	while (evs->quitEvent == false && SDL_PollEvent(&e) != 0) {
		switch (e.type) {
		case SDL_QUIT:
			evs->quitEvent = true;
			break;
		case SDL_WINDOWEVENT:
			switch (e.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				evs->windowResizeEvent = true;
				evs->windowDims = (Vec2I){ e.window.data1 , e.window.data2 };
				break;
			}
			break;
		case SDL_KEYDOWN:
			if (e.key.repeat == 0) {
				evs->keyDownEvent = true;
				evs->keysPressed[KeyFromSDLScancode(e.key.keysym.scancode)] += 1;
			}
			break;
		case SDL_KEYUP:
			if (e.key.repeat == 0) {
				evs->keyUpEvent = true;
				evs->keysReleased[KeyFromSDLScancode(e.key.keysym.scancode)] += 1;
			}
			break;
		case SDL_MOUSEMOTION:
			evs->mouseMotionEvent = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			evs->mouseButtonDownEvent = true;
			evs->buttonsPressed[ButtonFromSDLButton(e.button.button)] += 1;
			break;
		case SDL_MOUSEBUTTONUP:
			evs->mouseButtonUpEvent = true;
			evs->buttonsReleased[ButtonFromSDLButton(e.button.button)] += 1;
			break;
		case SDL_MOUSEWHEEL:
			evs->mouseWheelEvent = true;
			if (0 < e.wheel.y) {
				evs->buttonsPressed[BUTTON_SCROLL_UP] += (int16_t)e.wheel.y;
			} else {
				evs->buttonsPressed[BUTTON_SCROLL_DOWN] += -(int16_t)e.wheel.y;
			}
			break;
		case SDL_TEXTINPUT:
			if (SDL_IsTextInputActive()) {
				evs->textInputEvent = true;
				strcat(evs->textInput, e.text.text);
			}
			break;
		default:
			break;
		}
	}

	int keyCount = 0;
	const uint8_t* keyboardState = SDL_GetKeyboardState(&keyCount);
	for (int i = 0; i < _KEY_COUNT; i++) {
		const SDL_Scancode scancode = SDLScancodeFromKey(i);
		if (scancode != SDL_SCANCODE_UNKNOWN) {
			evs->keyStates[i] = keyboardState[scancode];
		}
	}
	for (int i = 0; i < keyCount; i++) {
		evs->rawKeyStates[i] = keyboardState[i];
	}

	const uint32_t mouseStateBitmask = SDL_GetMouseState(&evs->mousePosition.x, &evs->mousePosition.y);
	if (mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_LEFT)) {
		evs->buttonStates[BUTTON_PRIMARY] = 1;
	}
	if (mouseStateBitmask & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
		evs->buttonStates[BUTTON_SECONDARY] = 1;
	}

	return evs->quitEvent || evs->windowResizeEvent || evs->keyDownEvent || evs->keyUpEvent || evs->mouseMotionEvent || evs->mouseButtonDownEvent || evs->mouseWheelEvent || evs->mouseButtonUpEvent || evs->textInputEvent;
}
