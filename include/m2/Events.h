#pragma once
#include <m2/common/math/RectI.h>
#include <m2/common/math/VecI.h>
#include <m2/common/math/VecF.h>
#include <m2/protobuf/Detail.h>
#include <m2/common/Constants.h>
#include <m2/thirdparty/event/Event.h>
#include <m2g_KeyType.pb.h>
#include <array>
#include <optional>
#include <sstream>
#include <vector>

namespace m2 {
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
			VecF positionLpx;
		};
		/// For each button, only one actions is stored per loop
		std::array<MouseAction, U(MouseButton::end)> _mouseActions{};

		int32_t _verticalScrollCount{};
		int32_t _horizontalScrollCount{};

		struct Finger {
			thirdparty::event::FingerId id;
			VecF positionLpx;
		};
		std::vector<Finger> _fingerPresses;
		std::vector<Finger> _fingerReleases;
		std::vector<Finger> _fingerCancels;

		std::stringstream _textInput;

		// Persistent states

		std::vector<bool> _downKeys = std::vector<bool>(pb::enum_value_count<m2g::pb::KeyType>());
		std::array<bool, U(MouseButton::end)> _downButtons{};
		/// Mouse position in logical pixels
		VecF _mousePositionLpx;
		/// Mouse position in world coordinates. Non-zero only if there's an active level.
		VecF _mousePositionM;
		/// Currently-down fingers and their latest Lpx positions
		std::vector<Finger> _activeFingers;

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

		std::optional<VecF> PeekMouseButtonPress(MouseButton mb) const;
		bool PopMouseButtonPress(MouseButton mb);
		std::optional<VecF> PeekMouseButtonPress(MouseButton mb, const RectF& rectLpx) const;
		bool PopMouseButtonPress(MouseButton mb, const RectF& rectLpx);
		std::optional<VecF> PeekMouseButtonRelease(MouseButton mb) const;
		bool PopMouseButtonRelease(MouseButton mb);
		std::optional<VecF> PeekMouseButtonRelease(MouseButton mb, const RectF& rectLpx) const;
		bool PopMouseButtonRelease(MouseButton mb, const RectF& rectLpx);
		void ClearMouseButtonActions(const RectF& rectLpx);

		// Mouse scroll

		int32_t PopMouseWheelVerticalScroll();
		int32_t PopMouseWheelVerticalScroll(const RectF& rectLpx);
		int32_t PopMouseWheelHorizontalScroll();
		int32_t PopMouseWheelHorizontalScroll(const RectF& rectLpx);
		void ClearMouseWheelScrolls(const RectF& rectLpx);

		std::optional<std::string> PopTextInput();

		const std::vector<Finger>& ActiveFingers() const { return _activeFingers; }
		std::vector<Finger> PopFingerPresses();
		std::vector<Finger> PopFingerReleases();
		std::vector<Finger> PopFingerCancels();
		std::vector<Finger> PeekFingerPresses(const RectF& rectLpx) const;
		std::vector<Finger> PopFingerPresses(const RectF& rectLpx);
		std::vector<Finger> PeekFingerReleases(const RectF& rectLpx) const;
		std::vector<Finger> PopFingerReleases(const RectF& rectLpx);
		std::vector<Finger> PeekFingerCancels(const RectF& rectLpx) const;
		std::vector<Finger> PopFingerCancels(const RectF& rectLpx);
		void ClearFingerActions(const RectF& rectLpx);

		// Persistent states

		bool IsKeyDown(const m2g::pb::KeyType key) const { return _downKeys[pb::enum_index(key)]; }
		bool IsMouseButtonDown(const MouseButton mb) const { return _downButtons[U(mb)]; }
		void ClearMouseButtonDown(const RectF& rectLpx);
		VecF MousePositionLpx() const { return _mousePositionLpx; }
		VecF GetWorldPositionOfMouse() const { return _mousePositionM; }
	};
}
