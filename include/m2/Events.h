#pragma once
#include "math/RectI.h"
#include <m2/math/VecI.h>
#include <m2/math/VecF.h>
#include <m2/protobuf/Detail.h>
#include <m2g_KeyType.pb.h>
#include <array>
#include <optional>
#include <sstream>

namespace m2 {
	enum class MouseButton {
		PRIMARY,
		SECONDARY,
		MIDDLE,
		end
	};

	/// Stateful event manager
	class Events {
		bool _quit{};
		bool _windowResized{};
		std::vector<uint16_t> _keysPressed = std::vector<uint16_t>(pb::enum_value_count<m2g::pb::KeyType>());
		std::vector<uint16_t> _keysReleased = std::vector<uint16_t>(pb::enum_value_count<m2g::pb::KeyType>());

		enum class MouseActionType {
			NO_ACTION = 0,
			PRESSED,
			RELEASED
		};
		struct MouseAction {
			MouseActionType type{};
			VecI positionPx;
		};
		/// For each button, only one actions is stored per loop
		std::array<MouseAction, U(MouseButton::end)> _mouseActions{};

		int32_t _verticalScrollCount{};
		int32_t _horizontalScrollCount{};

		std::stringstream _textInput;

		// Persistent states

		std::vector<bool> _downKeys = std::vector<bool>(pb::enum_value_count<m2g::pb::KeyType>());
		std::array<bool, U(MouseButton::end)> _downButtons{};
		/// Mouse position in screen coordinates
		VecI _mousePositionPx;
		/// Mouse position in world coordinates. Non-zero only if there's an active level.
		VecF _mousePositionM;

	public:
		Events() = default;

		void Clear();
		bool Gather();

		// Global

		bool PopQuit();
		bool PopWindowResize();

		// Key presses

		bool PopKeyPress(m2g::pb::KeyType key);
		bool PopKeyRelease(m2g::pb::KeyType key);

		// Mouse button

		bool PeekMouseButtonPress(MouseButton mb) const;
		bool PopMouseButtonPress(MouseButton mb);
		bool PeekMouseButtonPress(MouseButton mb, const RectI& rect) const;
		bool PopMouseButtonPress(MouseButton mb, const RectI& rect);
		bool PeekMouseButtonRelease(MouseButton mb) const;
		bool PopMouseButtonRelease(MouseButton mb);
		bool PeekMouseButtonRelease(MouseButton mb, const RectI& rect) const;
		bool PopMouseButtonRelease(MouseButton mb, const RectI& rect);
		void ClearMouseButtonActions(const RectI& rect);

		// Mouse scroll

		int32_t PopMouseWheelVerticalScroll();
		int32_t PopMouseWheelVerticalScroll(const RectI& rect);
		int32_t PopMouseWheelHorizontalScroll();
		int32_t PopMouseWheelHorizontalScroll(const RectI& rect);
		void ClearMouseWheelScrolls(const RectI& rect);

		std::optional<std::string> PopTextInput();

		// Persistent states

		bool IsKeyDown(const m2g::pb::KeyType key) const { return _downKeys[pb::enum_index(key)]; }
		bool IsMouseButtonDown(const MouseButton mb) const { return _downButtons[U(mb)]; }
		void ClearMouseButtonDown(const RectI& rect);
		VecI MousePosition() const { return _mousePositionPx; }
		VecF GetWorldPositionOfMouse() const { return _mousePositionM; }
	};
}
