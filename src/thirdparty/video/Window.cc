#include <m2/thirdparty/video/Window.h>
#include <m2/Game.h>
#include <m2/common/Error.h>
#include <SDL2/SDL.h>

m2::expected<m2::thirdparty::video::Window> m2::thirdparty::video::Window::Create(
		const VecI minDimensions, const char* title) {
	auto* window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		minDimensions.x, minDimensions.y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (not window) {
		return make_unexpected(std::string{SDL_GetError()});
	}
	SDL_SetWindowMinimumSize(window, minDimensions.x, minDimensions.y);
	return Window{window};
}

m2::thirdparty::video::Window::Window(Window&& other) noexcept : _window(other._window) {
	other._window = nullptr;
}
m2::thirdparty::video::Window& m2::thirdparty::video::Window::operator=(Window&& other) noexcept {
	std::swap(_window, other._window);
	return *this;
}
m2::thirdparty::video::Window::~Window() {
	if (_window) {
		SDL_DestroyWindow(static_cast<SDL_Window*>(_window));
		_window = nullptr;
	}
}

uint32_t m2::thirdparty::video::Window::GetPixelFormat() const {
	if (const auto pixelFormat = SDL_GetWindowPixelFormat(static_cast<SDL_Window*>(_window));
			pixelFormat == SDL_PIXELFORMAT_UNKNOWN) {
		throw M2_ERROR("SDL error: " + std::string{SDL_GetError()});
	} else {
		return pixelFormat;
	}
}
