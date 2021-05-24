#include "EventHandling.h"

uint8_t gKeysPressed[_KEY_COUNT];
uint8_t gKeysReleased[_KEY_COUNT];
uint8_t gKeysState[_KEY_COUNT];

void GatherEvents(bool *outQuit) {
	// Clear events
	memset(gKeysPressed, 0, sizeof(gKeysPressed));
	memset(gKeysReleased, 0, sizeof(gKeysReleased));
	memset(gKeysState, 0, sizeof(gKeysState));
	// Handle events
	bool quit = false;
	SDL_Event e;
	while (!quit && SDL_PollEvent(&e) != 0) {
		switch (e.type) {
		case SDL_QUIT:
			quit = true;
			break;
		case SDL_WINDOWEVENT:
			//res = on_event_window(delta_time, &e.window);
			break;
		case SDL_KEYDOWN:
			if (e.key.repeat == 0) {
				gKeysPressed[KeyFromSDLScancode(e.key.keysym.scancode)] = 1;
			}
			break;
		case SDL_KEYUP:
			if (e.key.repeat == 0) {
				gKeysReleased[KeyFromSDLScancode(e.key.keysym.scancode)] = 1;
			}
			break;
		case SDL_MOUSEMOTION:
			//res = on_event_mouse_motion(delta_time, &e.motion);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			//res = on_event_mouse_button(delta_time, &e.button);
			break;
		case SDL_MOUSEWHEEL:
			//res = on_event_mouse_wheel(delta_time, &e.wheel);
			break;
		default:
			break;
		}
	}
	KeyStateArrayFillFromSDLKeyboardStateArray(gKeysState, SDL_GetKeyboardState(NULL));
	*outQuit = quit;
}

bool IsKeyPressed(Key key) {
	return gKeysPressed[key];
}

bool IsKeyReleased(Key key) {
	return gKeysReleased[key];
}

bool IsKeyDown(Key key) {
	return gKeysState[key];
}
