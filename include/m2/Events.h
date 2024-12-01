#pragma once
#include <m2/Controls.h>
#include "math/RectI.h"
#include <m2/math/VecI.h>
#include <m2/math/VecF.h>
#include <array>
#include <optional>
#include <sstream>
#include <array>
#include <deque>

namespace m2 {
	/// Stateful event manager
	class Events {
		static constexpr unsigned ui_key_press_count_limit = 16;

		bool quit{};

		std::optional<m2::VecI> window_resize;

		uint32_t key_press_count{};
		std::array<uint16_t, u(Key::end)> keys_pressed{};
		std::deque<SDL_Scancode> ui_keys_pressed;

		uint32_t key_release_count{};
		std::array<uint16_t, u(Key::end)> keys_released{};

		uint32_t mouse_button_press_count{};
		std::array<uint16_t, u(MouseButton::end)> mouse_buttons_pressed{};

		uint32_t mouse_button_release_count{};
		std::array<uint16_t, u(MouseButton::end)> mouse_buttons_released{};

		int32_t mouse_wheel_vertical_scroll_count{};
		int32_t mouse_wheel_horizontal_scroll_count{};

		std::stringstream text_input;

		// Persistent states
		std::array<bool, SDL_NUM_SCANCODES> sdl_keys_down{};
		std::array<bool, u(Key::end)> keys_down{};
		std::array<bool, u(MouseButton::end)> mouse_buttons_down{};
		m2::VecI _mouse_position;

	public:
		Events() = default;

		void clear();
		bool gather();

		// Quit
		bool pop_quit();
		// Window resize
		std::optional<VecI> pop_window_resize();
		// Key presses
		bool pop_key_press(Key k);
		bool pop_ui_key_press(SDL_Scancode scode);
		bool pop_key_release(Key k);
		// Mouse button
		bool peek_mouse_button_press(MouseButton mb);
		bool pop_mouse_button_press(MouseButton mb);
		bool peek_mouse_button_press(MouseButton mb, const RectI& rect);
		bool pop_mouse_button_press(MouseButton mb, const RectI& rect);
		void clear_mouse_button_presses(const RectI& rect);
		bool peek_mouse_button_release(MouseButton mb);
		bool pop_mouse_button_release(MouseButton mb);
		bool peek_mouse_button_release(MouseButton mb, const RectI& rect);
		bool pop_mouse_button_release(MouseButton mb, const RectI& rect);
		void clear_mouse_button_releases(const RectI& rect);
		// Mouse scroll
		int32_t pop_mouse_wheel_vertical_scroll();
		int32_t pop_mouse_wheel_vertical_scroll(const RectI& rect);
		int32_t pop_mouse_wheel_horizontal_scroll();
		int32_t pop_mouse_wheel_horizontal_scroll(const RectI& rect);
		void clear_mouse_wheel_scrolls(const RectI& rect);
		// Text input
		std::optional<std::string> pop_text_input();

		// Continuous states
		bool is_sdl_key_down(SDL_Scancode sc) const;
		bool is_key_down(Key k) const;
		bool is_mouse_button_down(MouseButton mb) const;
		void clear_mouse_button_down(const RectI& rect);
		VecI mouse_position() const;

		// Selection
		std::pair<std::optional<VecF>, std::optional<VecF>> primary_selection_position_m() const;
		std::pair<std::optional<VecF>, std::optional<VecF>> secondary_selection_position_m() const;
		// Selection management
		static void enable_primary_selection(const RectI& screen_rect);
		static void enable_secondary_selection(const RectI& screen_rect);
		static void reset_primary_selection();
		static void reset_secondary_selection();
		static void disable_primary_selection();
		static void disable_secondary_selection();
	};
}
