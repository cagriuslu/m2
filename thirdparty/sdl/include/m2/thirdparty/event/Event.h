#pragma once
#include <m2/common/Constants.h>
#include <m2/common/math/VecF.h>
#include <m2/common/math/VecI.h>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace m2::thirdparty::event {
	using Scancode = int32_t;
	constexpr Scancode UnknownScancode = 0;
	using FingerId = int64_t;   // maps from SDL_FingerID (Uint64); finger ids are small positive values in practice

	struct QuitEvent {};
	struct WindowResizeEvent {};
	struct KeyDownEvent   { Scancode scancode; bool isRepeat; };
	struct KeyUpEvent     { Scancode scancode; bool isRepeat; };
	struct MouseMotionEvent     { VecF positionLpx; };
	struct MouseButtonDownEvent { MouseButton button; VecF positionLpx; };
	struct MouseButtonUpEvent   { MouseButton button; VecF positionLpx; };
	struct MouseWheelEvent      { int32_t horizontal; int32_t vertical; };
	struct TextInputEvent       { std::string text; };
	struct FingerDownEvent   { FingerId finger; VecF positionLpx; };
	struct FingerMotionEvent { FingerId finger; VecF positionLpx; };
	struct FingerUpEvent     { FingerId finger; VecF positionLpx; };
	struct FingerCancelEvent { FingerId finger; VecF positionLpx; };

	using Event = std::variant<
		QuitEvent, WindowResizeEvent, KeyDownEvent, KeyUpEvent,
		MouseMotionEvent, MouseButtonDownEvent, MouseButtonUpEvent,
		MouseWheelEvent, TextInputEvent,
		FingerDownEvent, FingerMotionEvent, FingerUpEvent, FingerCancelEvent>;

	std::optional<Event> PollEvent();

	bool IsScancodeDown(Scancode scancode);

	void StartTextInput(void* sdlWindow);
	void StopTextInput(void* sdlWindow);
	bool IsTextInputActive(void* sdlWindow);

	std::optional<MouseButton> SystemButtonToMouseButton(int sdlButton);
	VecF GetMousePosition();
	bool IsMouseButtonDown(MouseButton button);
	std::vector<std::pair<FingerId, VecF>> GetActiveFingers(void* sdlWindow);
}
