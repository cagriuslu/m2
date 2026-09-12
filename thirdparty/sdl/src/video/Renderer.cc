#include <m2/thirdparty/video/Renderer.h>
#include <m2/thirdparty/video/Window.h>
#include "SdlConversions.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <vector>

namespace {
	/// Remembered in place of the previous viewport when no explicit viewport was previously set.
	const m2::RectI WHOLE_RENDER_TARGET{0, 0, -1, -1};

	m2::RectI GetCurrentViewport(SDL_Renderer* renderer) {
		if (not SDL_RenderViewportSet(renderer)) {
			return WHOLE_RENDER_TARGET;
		}
		SDL_Rect viewport;
		if (not SDL_GetRenderViewport(renderer, &viewport)) {
			throw M2_ERROR(std::string{"SDL_GetRenderViewport error: "} + SDL_GetError());
		}
		return m2::RectI{viewport.x, viewport.y, viewport.w, viewport.h};
	}
}

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

void m2::thirdparty::video::Renderer::DrawLineStrip(const std::span<const VecF> pointsLpx, const RGBA& color) {
	const auto pixelsPerUnit = GetPixelsPerWindowUnit();

	auto* sdlRenderer = static_cast<SDL_Renderer*>(_renderer);
	if (not SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a)) {
		throw M2_ERROR(std::string{"SDL_SetRenderDrawColor error: "} + SDL_GetError());
	}
	std::vector<SDL_FPoint> sdlPoints;
	sdlPoints.reserve(pointsLpx.size());
	for (const auto& point : pointsLpx) {
		sdlPoints.push_back(ToSdlFPoint(point.Scale(pixelsPerUnit)));
	}
	if (not SDL_RenderLines(sdlRenderer, sdlPoints.data(), I(sdlPoints.size()))) {
		throw M2_ERROR(std::string{"SDL_RenderLines error: "} + SDL_GetError());
	}
}

m2::thirdparty::video::Renderer::ViewportGuard::ViewportGuard(void* renderer, const RectI& viewportPx)
		: _renderer(renderer), _previousViewportPx(GetCurrentViewport(static_cast<SDL_Renderer*>(renderer))) {
	const auto sdlViewport = ToSdlRect(viewportPx);
	if (not SDL_SetRenderViewport(static_cast<SDL_Renderer*>(_renderer), &sdlViewport)) {
		throw M2_ERROR(std::string{"SDL_SetRenderViewport error: "} + SDL_GetError());
	}
}
m2::thirdparty::video::Renderer::ViewportGuard::ViewportGuard(ViewportGuard&& other) noexcept
		: _renderer(other._renderer), _previousViewportPx(other._previousViewportPx) {
	other._renderer = nullptr;
}
m2::thirdparty::video::Renderer::ViewportGuard& m2::thirdparty::video::Renderer::ViewportGuard::operator=(ViewportGuard&& other) noexcept {
	std::swap(_renderer, other._renderer);
	std::swap(_previousViewportPx, other._previousViewportPx);
	return *this;
}
m2::thirdparty::video::Renderer::ViewportGuard::~ViewportGuard() {
	if (_renderer) {
		const auto sdlPreviousViewport = ToSdlRect(_previousViewportPx);
		const auto* const restoredViewport = _previousViewportPx == WHOLE_RENDER_TARGET ? nullptr : &sdlPreviousViewport;
		if (not SDL_SetRenderViewport(static_cast<SDL_Renderer*>(_renderer), restoredViewport)) {
			// A destructor must not throw, but the failure must not pass unnoticed either: every later draw
			// would be clipped and translated by this guard's viewport instead of the restored one.
			std::cerr << "SDL_SetRenderViewport error while restoring the previous viewport: " << SDL_GetError() << std::endl;
		}
	}
}
m2::thirdparty::video::Renderer::ViewportGuard m2::thirdparty::video::Renderer::ScopedViewport(const RectI& viewportPx) {
	return ViewportGuard{_renderer, viewportPx};
}
