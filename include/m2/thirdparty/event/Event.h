#pragma once
#include <m2/Events.h>
#include <m2/math/VecI.h>
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
	struct MouseMotionEvent     { VecI positionPx; };
	struct MouseButtonDownEvent { MouseButton button; VecI positionPx; };
	struct MouseButtonUpEvent   { MouseButton button; VecI positionPx; };
	struct MouseWheelEvent      { int32_t horizontal; int32_t vertical; };
	struct TextInputEvent       { std::string text; };

	using Event = std::variant<
		QuitEvent, WindowResizeEvent, KeyDownEvent, KeyUpEvent,
		MouseMotionEvent, MouseButtonDownEvent, MouseButtonUpEvent,
		MouseWheelEvent, TextInputEvent>;

	std::optional<Event> PollEvent();

	bool IsScancodeDown(Scancode scancode);

	void StartTextInput();
	void StopTextInput();
	bool IsTextInputActive();

	std::optional<MouseButton> SystemButtonToMouseButton(int sdlButton);
	VecI GetMousePosition();
	bool IsMouseButtonDown(MouseButton button);
}
