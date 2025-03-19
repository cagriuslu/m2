#pragma once
#include <m2/Controls.h>
#include "math/RectI.h"
#include <m2/math/VecI.h>
#include <m2/math/VecF.h>
#include <m2/protobuf/Detail.h>
#include <m2g_KeyType.pb.h>
#include <array>
#include <optional>
#include <sstream>
#include <array>
#include <deque>

namespace m2 {
	/// Stateful event manager
	class Events {
		bool quit{};

		bool window_resize{};

		uint32_t key_press_count{};
		std::vector<uint16_t> keys_pressed = std::vector<uint16_t>(pb::enum_value_count<m2g::pb::KeyType>());

		uint32_t key_release_count{};
		std::vector<uint16_t> keys_released = std::vector<uint16_t>(pb::enum_value_count<m2g::pb::KeyType>());

		uint32_t mouse_button_press_count{};
		std::array<uint16_t, u(MouseButton::end)> mouse_buttons_pressed{};

		uint32_t mouse_button_release_count{};
		std::array<uint16_t, u(MouseButton::end)> mouse_buttons_released{};

		int32_t mouse_wheel_vertical_scroll_count{};
		int32_t mouse_wheel_horizontal_scroll_count{};

		std::stringstream text_input;

		// Persistent states
		std::vector<bool> keys_down = std::vector<bool>(pb::enum_value_count<m2g::pb::KeyType>());
		std::array<bool, u(MouseButton::end)> mouse_buttons_down{};
		VecI _mouse_position;

	public:
		Events() = default;

		void clear();
		bool gather();

		// Quit
		bool pop_quit();
		// Window resize
		bool pop_window_resize();
		// Key presses
		bool pop_key_press(m2g::pb::KeyType key);
		bool pop_key_release(m2g::pb::KeyType key);
		// Mouse button
		bool peek_mouse_button_press(MouseButton mb) const;
		bool pop_mouse_button_press(MouseButton mb);
		bool peek_mouse_button_press(MouseButton mb, const RectI& rect) const;
		bool pop_mouse_button_press(MouseButton mb, const RectI& rect);
		void clear_mouse_button_presses(const RectI& rect);
		bool peek_mouse_button_release(MouseButton mb) const;
		bool pop_mouse_button_release(MouseButton mb);
		bool peek_mouse_button_release(MouseButton mb, const RectI& rect) const;
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
		bool is_key_down(m2g::pb::KeyType key) const;
		bool is_mouse_button_down(MouseButton mb) const;
		void clear_mouse_button_down(const RectI& rect);
		VecI mouse_position() const;
	};
}
