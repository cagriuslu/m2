#include <m2/third_party/video/Mouse.h>
#include <SDL2/SDL.h>

std::optional<m2::MouseButton> m2::thirdparty::video::SystemButtonToMouseButton(const int button) {
	switch (button) {
		case SDL_BUTTON_LEFT:
			return MouseButton::PRIMARY;
		case SDL_BUTTON_RIGHT:
			return MouseButton::SECONDARY;
		case SDL_BUTTON_MIDDLE:
			return MouseButton::MIDDLE;
		default:
			return std::nullopt;
	}
}
