#ifndef M2_CONTROLS_H
#define M2_CONTROLS_H

#include <SDL.h>

namespace m2 {
	enum class Key {
		UNKNOWN = 0,
		MENU,
		UP,
		DOWN,
		LEFT,
		RIGHT,
		DASH, // TODO rename to SPACE
		CONSOLE,
		ENTER,
		BACKSPACE,
		end
	};
	constexpr unsigned u(Key k) {
		return static_cast<unsigned>(k);
	}

	enum class MouseButton {
		UNKNOWN = 0,
		PRIMARY,
		SECONDARY,
		MIDDLE,
		end
	};
	constexpr unsigned u(MouseButton mb) {
		return static_cast<unsigned>(mb);
	}
	MouseButton button_to_mouse_button(int button);
}

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

#endif //M2_CONTROLS_H
