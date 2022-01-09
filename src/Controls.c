#include "Controls.h"

Key KeyFromSDLScancode(SDL_Scancode sc) {
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
	case SDL_SCANCODE_LSHIFT:
		return KEY_MODIFIER_SHIFT;
	case SDL_SCANCODE_GRAVE:
		return KEY_CONSOLE;
	case SDL_SCANCODE_RETURN:
		return KEY_ENTER;
	default:
		return KEY_NONE;
	}
}

SDL_Scancode SDLScancodeFromKey(Key k) {
	switch (k) {
		case KEY_MENU:
			return SDL_SCANCODE_ESCAPE;
		case KEY_UP:
			return SDL_SCANCODE_W;
		case KEY_DOWN:
			return SDL_SCANCODE_S;
		case KEY_LEFT:
			return SDL_SCANCODE_A;
		case KEY_RIGHT:
			return SDL_SCANCODE_D;
		case KEY_MODIFIER_SHIFT:
			return SDL_SCANCODE_LSHIFT;
		case KEY_CONSOLE:
			return SDL_SCANCODE_GRAVE;
		case KEY_ENTER:
			return SDL_SCANCODE_RETURN;
		default:
			return SDL_SCANCODE_UNKNOWN;
	}
}

MouseButton ButtonFromSDLButton(int button) {
	switch (button) {
	case SDL_BUTTON_LEFT:
		return BUTTON_PRIMARY;
	case SDL_BUTTON_RIGHT:
		return BUTTON_SECONDARY;
	case SDL_BUTTON_MIDDLE:
		return BUTTON_MIDDLE;
	default:
		return BUTTON_NONE;
	}
}
