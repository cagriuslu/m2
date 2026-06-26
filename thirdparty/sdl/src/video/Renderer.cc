#include <m2/thirdparty/video/Renderer.h>
#include <m2/thirdparty/video/Window.h>
#include "SdlConversions.h"
#include <SDL3/SDL.h>
#include <vector>

m2::thirdparty::video::Renderer::Renderer(Renderer&& other) noexcept : _window(other._window), _renderer(other._renderer) {
	other._window = nullptr;
	other._renderer = nullptr;
}
m2::thirdparty::video::Renderer& m2::thirdparty::video::Renderer::operator=(Renderer&& other) noexcept {
	std::swap(_window, other._window);
	std::swap(_renderer, other._renderer);
	return *this;
}
m2::thirdparty::video::Renderer::~Renderer() {
	if (_renderer) {
		SDL_DestroyRenderer(static_cast<SDL_Renderer*>(_renderer));
		_renderer = nullptr;
	}
}

m2::VecF m2::thirdparty::video::Renderer::GetPixelsPerWindowUnit() const {
	int pixelsX, pixelsY;
	if (not SDL_GetWindowSizeInPixels(static_cast<SDL_Window*>(_window), &pixelsX, &pixelsY)) {
		throw M2_ERROR(std::format("SDL_GetWindowSizeInPixels error: {}", SDL_GetError()));
	}

	int x,y;
	if (not SDL_GetWindowSize(static_cast<SDL_Window*>(_window), &x, &y)) {
		throw M2_ERROR(std::format("SDL_GetWindowSize error: {}", SDL_GetError()));
	}

	return {static_cast<float>(pixelsX) / static_cast<float>(x), static_cast<float>(pixelsY) / static_cast<float>(y)};
}
std::string m2::thirdparty::video::Renderer::GetName() const {
	const char* name = SDL_GetRendererName(static_cast<SDL_Renderer*>(_renderer));
	return name ? std::string{name} : std::string{};
}

void m2::thirdparty::video::Renderer::SetDrawColor(const RGBA& color) {
	SDL_SetRenderDrawColor(static_cast<SDL_Renderer*>(_renderer), color.r, color.g, color.b, color.a);
}
void m2::thirdparty::video::Renderer::Clear() {
	SDL_RenderClear(static_cast<SDL_Renderer*>(_renderer));
}
void m2::thirdparty::video::Renderer::Present() {
	SDL_RenderPresent(static_cast<SDL_Renderer*>(_renderer));
}

void m2::thirdparty::video::Renderer::DrawLineStrip(const std::span<const VecF> points, const RGBA& color) {
	const auto pixelsPerUnit = GetPixelsPerWindowUnit();

	auto* sdlRenderer = static_cast<SDL_Renderer*>(_renderer);
	if (not SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a)) {
		throw M2_ERROR(std::string{"SDL_SetRenderDrawColor error: "} + SDL_GetError());
	}
	std::vector<SDL_FPoint> sdlPoints;
	sdlPoints.reserve(points.size());
	for (const auto& point : points) {
		sdlPoints.push_back(ToSdlFPoint(point.Scale(pixelsPerUnit)));
	}
	if (not SDL_RenderLines(sdlRenderer, sdlPoints.data(), I(sdlPoints.size()))) {
		throw M2_ERROR(std::string{"SDL_RenderLines error: "} + SDL_GetError());
	}
}
