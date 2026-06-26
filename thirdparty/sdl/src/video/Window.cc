#include <m2/thirdparty/video/Window.h>
#include <m2/common/Error.h>
#include <SDL3/SDL.h>
#include <format>
#include <iostream>

using namespace m2;
using namespace m2::thirdparty;
using namespace m2::thirdparty::video;

m2::expected<m2::thirdparty::video::Window> m2::thirdparty::video::Window::Create(
		const VecI minDimensions, const char* title) {
	auto* window = SDL_CreateWindow(title, minDimensions.x, minDimensions.y, SDL_WINDOW_RESIZABLE);
	if (not window) {
		return make_unexpected(std::string{SDL_GetError()});
	}
	SDL_SetWindowMinimumSize(window, minDimensions.x, minDimensions.y);
	return Window{window};
}
m2::expected<std::pair<Window,Renderer>> Window::Create2(const VecI minDimensions, const char* title) {
	SDL_Window* window;
	SDL_Renderer* renderer;
	// TODO SDL_WINDOW_HIGH_PIXEL_DENSITY
	if (not SDL_CreateWindowAndRenderer(title, minDimensions.x, minDimensions.y, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
		return make_unexpected(std::format("SDL_CreateWindowAndRenderer error: {}", SDL_GetError()));
	}
	SDL_SetWindowMinimumSize(window, minDimensions.x, minDimensions.y);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	int x,y;
	SDL_GetWindowSize(window, &x, &y);
	std::cerr << "SDL_GetWindowSize: " << x << "," << y << std::endl;
	SDL_GetWindowSizeInPixels(window, &x, &y);
	std::cerr << "SDL_GetWindowSizeInPixels: " << x << "," << y << std::endl;
	const auto scale = SDL_GetWindowDisplayScale(window);
	std::cerr << "SDL_GetWindowDisplayScale: " << scale << std::endl;
	const auto density = SDL_GetWindowPixelDensity(window);
	std::cerr << "SDL_GetWindowPixelDensity: " << density << std::endl;

	return std::make_pair(Window{window}, Renderer{renderer});
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
