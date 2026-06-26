#pragma once
#include <m2/common/Constants.h>
#include <m2/common/math/VecF.h>
#include <m2/common/math/VecI.h>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

namespace m2::thirdparty::event {
	using Scancode = int32_t;
	constexpr Scancode UnknownScancode = 0;

	struct QuitEvent {};
	struct WindowResizeEvent {};
	struct KeyDownEvent   { Scancode scancode; bool isRepeat; };
	struct KeyUpEvent     { Scancode scancode; bool isRepeat; };
	struct MouseMotionEvent     { VecF positionPx; };
	struct MouseButtonDownEvent { MouseButton button; VecF positionPx; };
	struct MouseButtonUpEvent   { MouseButton button; VecF positionPx; };
	struct MouseWheelEvent      { int32_t horizontal; int32_t vertical; };
	struct TextInputEvent       { std::string text; };

	using Event = std::variant<
		QuitEvent, WindowResizeEvent, KeyDownEvent, KeyUpEvent,
		MouseMotionEvent, MouseButtonDownEvent, MouseButtonUpEvent,
		MouseWheelEvent, TextInputEvent>;

	std::optional<Event> PollEvent();

	bool IsScancodeDown(Scancode scancode);

	void StartTextInput(void* sdlWindow);
	void StopTextInput(void* sdlWindow);
	bool IsTextInputActive(void* sdlWindow);

	std::optional<MouseButton> SystemButtonToMouseButton(int sdlButton);
	VecF GetMousePosition();
	bool IsMouseButtonDown(MouseButton button);
}
