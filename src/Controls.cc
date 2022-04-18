#include "m2/Controls.h"

unsigned m2::u(Key k) {
	return static_cast<unsigned>(k);
}

unsigned m2::u(MouseButton mb) {
	return static_cast<unsigned>(mb);
}

m2::MouseButton m2::button_to_mouse_button(int button) {
	switch (button) {
		case SDL_BUTTON_LEFT:
			return MouseButton::PRIMARY;
		case SDL_BUTTON_RIGHT:
			return MouseButton::SECONDARY;
		case SDL_BUTTON_MIDDLE:
			return MouseButton::MIDDLE;
		default:
			return MouseButton::UNKNOWN;
	}
}
