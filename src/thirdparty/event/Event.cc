#include <m2/thirdparty/event/Event.h>
#include <SDL2/SDL.h>

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
	while (SDL_PollEvent(&sdlEvent) != 0) {
		switch (sdlEvent.type) {
			case SDL_QUIT:
				return QuitEvent{};
			case SDL_WINDOWEVENT:
				if (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED
						|| sdlEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					return WindowResizeEvent{};
				}
				break;
			case SDL_KEYDOWN:
				return KeyDownEvent{static_cast<Scancode>(sdlEvent.key.keysym.scancode),
						sdlEvent.key.repeat != 0};
			case SDL_KEYUP:
				return KeyUpEvent{static_cast<Scancode>(sdlEvent.key.keysym.scancode),
						sdlEvent.key.repeat != 0};
			case SDL_MOUSEMOTION:
				return MouseMotionEvent{VecI{sdlEvent.motion.x, sdlEvent.motion.y}};
			case SDL_MOUSEBUTTONDOWN:
				if (const auto button = SystemButtonToMouseButton(sdlEvent.button.button)) {
					return MouseButtonDownEvent{*button, VecI{sdlEvent.button.x, sdlEvent.button.y}};
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (const auto button = SystemButtonToMouseButton(sdlEvent.button.button)) {
					return MouseButtonUpEvent{*button, VecI{sdlEvent.button.x, sdlEvent.button.y}};
				}
				break;
			case SDL_MOUSEWHEEL:
				return MouseWheelEvent{sdlEvent.wheel.x, sdlEvent.wheel.y};
			case SDL_TEXTINPUT: {
				std::string text;
				for (const char character : sdlEvent.text.text) {
					if (character == 0) { break; }
					text += character;
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
	const uint8_t* const keyboardState = SDL_GetKeyboardState(&keyCount);
	if (scancode < 0 || scancode >= keyCount) { return false; }
	return keyboardState[scancode] != 0;
}

void m2::thirdparty::event::StartTextInput()    { SDL_StartTextInput(); }
void m2::thirdparty::event::StopTextInput()     { SDL_StopTextInput(); }
bool m2::thirdparty::event::IsTextInputActive() { return SDL_IsTextInputActive() != SDL_FALSE; }

m2::VecI m2::thirdparty::event::GetMousePosition() {
	VecI position;
	SDL_GetMouseState(&position.x, &position.y);
	return position;
}

bool m2::thirdparty::event::IsMouseButtonDown(const m2::MouseButton button) {
	const uint32_t mouseStateBitmask = SDL_GetMouseState(nullptr, nullptr);
	return (mouseStateBitmask & SDL_BUTTON(MouseButtonToSystemButton(button))) != 0;
}
