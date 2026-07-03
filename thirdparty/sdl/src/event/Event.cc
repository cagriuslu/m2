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

	// Convert SDL's normalized [0,1] finger position to logical pixels using the finger event's window.
	// Returns std::nullopt if the window can't be resolved (should not happen for a single-window app;
	// guarded so we never emit a bogus (0,0) position).
	std::optional<m2::VecF> FingerPositionToLpx(const SDL_TouchFingerEvent& touchFingerEvent) {
		SDL_Window* const window = SDL_GetWindowFromID(touchFingerEvent.windowID);
		if (not window) { return std::nullopt; }
		int windowWidthLpx = 0, windowHeightLpx = 0;
		SDL_GetWindowSize(window, &windowWidthLpx, &windowHeightLpx);
		return m2::VecF{touchFingerEvent.x * static_cast<float>(windowWidthLpx),
				touchFingerEvent.y * static_cast<float>(windowHeightLpx)};
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
			case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED:
			case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
			case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
				return WindowResizeEvent{};
			case SDL_EVENT_KEY_DOWN:
				return KeyDownEvent{static_cast<Scancode>(sdlEvent.key.scancode),
						sdlEvent.key.repeat != 0};
			case SDL_EVENT_KEY_UP:
				return KeyUpEvent{static_cast<Scancode>(sdlEvent.key.scancode),
						sdlEvent.key.repeat != 0};
			case SDL_EVENT_MOUSE_MOTION:
				return MouseMotionEvent{VecF{sdlEvent.motion.x, sdlEvent.motion.y}};
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				if (const auto button = SystemButtonToMouseButton(sdlEvent.button.button)) {
					return MouseButtonDownEvent{*button, VecF{sdlEvent.button.x, sdlEvent.button.y}};
				}
				break;
			case SDL_EVENT_MOUSE_BUTTON_UP:
				if (const auto button = SystemButtonToMouseButton(sdlEvent.button.button)) {
					return MouseButtonUpEvent{*button, VecF{sdlEvent.button.x, sdlEvent.button.y}};
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
			case SDL_EVENT_FINGER_DOWN:
				if (const auto positionLpx = FingerPositionToLpx(sdlEvent.tfinger)) {
					return FingerDownEvent{static_cast<FingerId>(sdlEvent.tfinger.fingerID), *positionLpx};
				}
				break;
			case SDL_EVENT_FINGER_MOTION:
				if (const auto positionLpx = FingerPositionToLpx(sdlEvent.tfinger)) {
					return FingerMotionEvent{static_cast<FingerId>(sdlEvent.tfinger.fingerID), *positionLpx};
				}
				break;
			case SDL_EVENT_FINGER_UP:
				if (const auto positionLpx = FingerPositionToLpx(sdlEvent.tfinger)) {
					return FingerUpEvent{static_cast<FingerId>(sdlEvent.tfinger.fingerID), *positionLpx};
				}
				break;
			case SDL_EVENT_FINGER_CANCELED:
				if (const auto positionLpx = FingerPositionToLpx(sdlEvent.tfinger)) {
					return FingerCancelEvent{static_cast<FingerId>(sdlEvent.tfinger.fingerID), *positionLpx};
				}
				break;
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

m2::VecF m2::thirdparty::event::GetMousePosition() {
	float x = 0.0f, y = 0.0f;
	SDL_GetMouseState(&x, &y);
	return VecF{x, y};
}

bool m2::thirdparty::event::IsMouseButtonDown(const m2::MouseButton button) {
	const SDL_MouseButtonFlags mouseStateBitmask = SDL_GetMouseState(nullptr, nullptr);
	return (mouseStateBitmask & SDL_BUTTON_MASK(MouseButtonToSystemButton(button))) != 0;
}

std::vector<std::pair<FingerId, m2::VecF>> m2::thirdparty::event::GetActiveFingers(void* const sdlWindow) {
	std::vector<std::pair<FingerId, m2::VecF>> activeFingers;
	auto* const window = static_cast<SDL_Window*>(sdlWindow);
	if (not window) { return activeFingers; }
	// Finger positions are normalized [0,1]; scale to logical pixels using the window size, matching
	// FingerPositionToLpx above (used by the event-based path).
	int windowWidthLpx = 0, windowHeightLpx = 0;
	SDL_GetWindowSize(window, &windowWidthLpx, &windowHeightLpx);
	int deviceCount = 0;
	SDL_TouchID* const touchDevices = SDL_GetTouchDevices(&deviceCount);
	if (not touchDevices) { return activeFingers; }
	for (int deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
		int fingerCount = 0;
		SDL_Finger** const fingers = SDL_GetTouchFingers(touchDevices[deviceIndex], &fingerCount);
		if (not fingers) { continue; }
		for (int fingerIndex = 0; fingerIndex < fingerCount; ++fingerIndex) {
			const SDL_Finger* const finger = fingers[fingerIndex];
			activeFingers.emplace_back(static_cast<FingerId>(finger->id),
					m2::VecF{finger->x * static_cast<float>(windowWidthLpx),
							finger->y * static_cast<float>(windowHeightLpx)});
		}
		SDL_free(fingers);
	}
	SDL_free(touchDevices);
	return activeFingers;
}
