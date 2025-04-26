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

		void Clear();
		bool Gather();

		// Quit
		bool PopQuit();
		// Window resize
		bool PopWindowResize();
		// Key presses
		bool PopKeyPress(m2g::pb::KeyType key);
		bool PopKeyRelease(m2g::pb::KeyType key);
		// Mouse button
		bool PeekMouseButtonPress(MouseButton mb) const;
		bool PopMouseButtonPress(MouseButton mb);
		bool PeekMouseButtonPress(MouseButton mb, const RectI& rect) const;
		bool PopMouseButtonPress(MouseButton mb, const RectI& rect);
		void ClearMouseButtonPresses(const RectI& rect);
		bool PeekMouseButtonRelease(MouseButton mb) const;
		bool PopMouseButtonRelease(MouseButton mb);
		bool PeekMouseButtonRelease(MouseButton mb, const RectI& rect) const;
		bool PopMouseButtonRelease(MouseButton mb, const RectI& rect);
		void ClearMouseButtonReleases(const RectI& rect);
		// Mouse scroll
		int32_t PopMouseWheelVerticalScroll();
		int32_t PopMouseWheelVerticalScroll(const RectI& rect);
		int32_t PopMouseWheelHorizontalScroll();
		int32_t PopMouseWheelHorizontalScroll(const RectI& rect);
		void ClearMouseWheelScrolls(const RectI& rect);
		// Text input
		std::optional<std::string> PopTextInput();

		// Continuous states

		bool IsKeyDown(m2g::pb::KeyType key) const;
		bool IsMouseButtonDown(MouseButton mb) const;
		void ClearMouseButtonDown(const RectI& rect);
		/// Position of the mouse in window coordinates where top left is (0,0).
		VecI MousePosition() const;
	};
}
