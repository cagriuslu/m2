#include <m2/thirdparty/event/Event.h>
#include <SDL3/SDL.h>

using namespace m2::thirdparty::event;

namespace {
	int MouseButtonToSystemButton(const m2::MouseButton button) {
		switch (button) {
			case m2::MouseButton::PRIMARY:   return SDL_BUTTON_LEFT;
			case m2::MouseButton::SECONDARY: return SDL_BUTTON_RIGHT;
			case m2::MouseButton::MIDDLE:    return SDL_BUTTON_MIDDLE;
			default:                         return 0;
		}
	}
}

std::optional<m2::MouseButton> m2::thirdparty::event::SystemButtonToMouseButton(const int sdlButton) {
	switch (sdlButton) {
		case SDL_BUTTON_LEFT:   return m2::MouseButton::PRIMARY;
		case SDL_BUTTON_RIGHT:  return m2::MouseButton::SECONDARY;
		case SDL_BUTTON_MIDDLE: return m2::MouseButton::MIDDLE;
		default:                return std::nullopt;
	}
}

std::optional<Event> m2::thirdparty::event::PollEvent() {
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent)) {
		switch (sdlEvent.type) {
			case SDL_EVENT_QUIT:
				return QuitEvent{};
			case SDL_EVENT_WINDOW_RESIZED:
			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
				return WindowResizeEvent{};
			case SDL_EVENT_KEY_DOWN:
				return KeyDownEvent{static_cast<Scancode>(sdlEvent.key.scancode),
						sdlEvent.key.repeat != 0};
			case SDL_EVENT_KEY_UP:
				return KeyUpEvent{static_cast<Scancode>(sdlEvent.key.scancode),
						sdlEvent.key.repeat != 0};
			case SDL_EVENT_MOUSE_MOTION:
				return MouseMotionEvent{VecI{static_cast<int>(sdlEvent.motion.x), static_cast<int>(sdlEvent.motion.y)}};
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				if (const auto button = SystemButtonToMouseButton(sdlEvent.button.button)) {
					return MouseButtonDownEvent{*button, VecI{static_cast<int>(sdlEvent.button.x), static_cast<int>(sdlEvent.button.y)}};
				}
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				if (const auto button = SystemButtonToMouseButton(sdlEvent.button.button)) {
					return MouseButtonUpEvent{*button, VecI{static_cast<int>(sdlEvent.button.x), static_cast<int>(sdlEvent.button.y)}};
				}
				break;
			case SDL_EVENT_MOUSE_WHEEL:
				return MouseWheelEvent{sdlEvent.wheel.integer_x, sdlEvent.wheel.integer_y};
			case SDL_EVENT_TEXT_INPUT: {
				std::string text;
				for (const char* character = sdlEvent.text.text; character && *character; ++character) {
					text += *character;
				}
				return TextInputEvent{std::move(text)};
			}
			default:
				break;
		}
	}
	return std::nullopt;
}

bool m2::thirdparty::event::IsScancodeDown(const Scancode scancode) {
	if (scancode == UnknownScancode) { return false; }
	int keyCount = 0;
	const bool* const keyboardState = SDL_GetKeyboardState(&keyCount);
	if (scancode < 0 || scancode >= keyCount) { return false; }
	return keyboardState[scancode];
}

void m2::thirdparty::event::StartTextInput(void* sdlWindow)    { SDL_StartTextInput(static_cast<SDL_Window*>(sdlWindow)); }
void m2::thirdparty::event::StopTextInput(void* sdlWindow)     { SDL_StopTextInput(static_cast<SDL_Window*>(sdlWindow)); }
bool m2::thirdparty::event::IsTextInputActive(void* sdlWindow) { return SDL_TextInputActive(static_cast<SDL_Window*>(sdlWindow)); }

m2::VecI m2::thirdparty::event::GetMousePosition() {
	float x = 0.0f, y = 0.0f;
	SDL_GetMouseState(&x, &y);
	return VecI{static_cast<int>(x), static_cast<int>(y)};
}

bool m2::thirdparty::event::IsMouseButtonDown(const m2::MouseButton button) {
	const SDL_MouseButtonFlags mouseStateBitmask = SDL_GetMouseState(nullptr, nullptr);
	return (mouseStateBitmask & SDL_BUTTON_MASK(MouseButtonToSystemButton(button))) != 0;
}
