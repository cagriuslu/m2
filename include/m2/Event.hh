#ifndef EVENT_HANDLING_H
#define EVENT_HANDLING_H

#include <m2/Controls.h>
#include <m2/Vec2i.h>

namespace m2 {
	struct Events {
		bool quit;

		bool window_resized;
		m2::Vec2i window_dimensions;

		bool key_pressed;
		uint16_t keys_pressed[static_cast<unsigned>(m2::Key::end)];

		bool key_released;
		uint16_t keys_released[static_cast<unsigned>(m2::Key::end)];

		bool mouse_moved;

		bool mouse_button_pressed;
		bool mouse_wheel_scrolled;
		uint16_t mouse_buttons_pressed[static_cast<unsigned>(m2::MouseButton::end)];

		bool mouse_button_released;
		uint16_t mouse_buttons_released[static_cast<unsigned>(m2::MouseButton::end)];

		bool text_input;
		char text[32];

		// Persistent states
		bool key_down[static_cast<unsigned>(m2::Key::end)];
		bool mouse_button_down[static_cast<unsigned>(m2::MouseButton::end)];
		m2::Vec2i mouse_position;

		uint8_t raw_key_down[SDL_NUM_SCANCODES];

		Events();
		bool gather();
	};
}

#endif
