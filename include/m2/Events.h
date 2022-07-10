#ifndef M2_EVENTS_H
#define M2_EVENTS_H

#include <m2/Controls.h>
#include <m2/Vec2i.h>
#include <array>
#include <optional>
#include <sstream>
#include <array>
#include <deque>

namespace m2 {
	class Events {
		static constexpr unsigned ui_key_press_count_limit = 16;

		bool quit;

		std::optional<m2::Vec2i> window_resize;

		uint32_t key_press_count;
		std::array<uint16_t, u(Key::end)> keys_pressed;
		std::deque<SDL_Scancode> ui_keys_pressed;

		uint32_t key_release_count;
		std::array<uint16_t, u(Key::end)> keys_released;

		uint32_t mouse_button_press_count;
		std::array<uint16_t, u(MouseButton::end)> mouse_buttons_pressed;

		uint32_t mouse_button_release_count;
		std::array<uint16_t, u(MouseButton::end)> mouse_buttons_released;

		uint32_t mouse_wheel_scroll_count;

		std::stringstream text_input;

		// Persistent states

		std::array<bool, SDL_NUM_SCANCODES> sdl_keys_down;
		std::array<bool, u(Key::end)> keys_down;
		std::array<bool, u(MouseButton::end)> mouse_buttons_down;
		m2::Vec2i _mouse_position;

	public:
		Events();

		void clear();
		bool gather();

		bool pop_quit();
		std::optional<Vec2i> pop_window_resize();
		bool pop_key_press(Key k);
		bool pop_ui_key_press(SDL_Scancode scode);
		bool pop_key_release(Key k);
		bool pop_mouse_button_press(MouseButton mb);
		bool pop_mouse_button_release(MouseButton mb);
		bool pop_mouse_wheel_scroll();
		std::optional<std::string> pop_text_input();

		bool is_sdl_key_down(SDL_Scancode sc) const;
		bool is_key_down(Key k) const;
		bool is_mouse_button_down(MouseButton mb) const;
		m2::Vec2i mouse_position() const;
	};
}

#endif
