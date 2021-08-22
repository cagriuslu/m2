#ifndef CONTROLS_H
#define CONTROLS_H

#include <SDL.h>

// Direct keys for Hud items
// Direct keys while navigating in the menus
// Ctrl+keys for save/load/direct menu actions
// Ctrl+keys and Ctrl+Alt+keys for Level Editor
// Try to only use 1-9-0 and a-z --> 36 keys
// It should be possible to do double key combinations, for example:
// Ctrl+Alt+1 followed by Ctrl+Alt+R --> 260 keys
// This can extend endlessly
// Ctrl+Alt+1 followed by Ctrl+Alt+1 followed by Ctrl+Alt+R --> 2600 keys
// Although, we're not have to connect each Key to a physical key combination, right?

typedef enum _Key {
	KEY_NONE = 0,

	KEY_MENU,

	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,

	KEY_MODIFIER_SHIFT,

	KEY_CONSOLE,
	KEY_ENTER,

	_KEY_COUNT
} Key;

typedef enum _Button {
	BUTTON_NONE = 0,

	BUTTON_PRIMARY,
	BUTTON_SECONDARY,
	BUTTON_SCROLL_DOWN,
	BUTTON_SCROLL_UP,

	_BUTTON_COUNT
} MouseButton;

Key KeyFromSDLScancode(SDL_Scancode sc);
SDL_Scancode SDLScancodeFromKey(Key k);

MouseButton ButtonFromSDLButton(int button);

#endif
