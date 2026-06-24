#include <m2/thirdparty/video/Renderer.h>
#include "SdlConversions.h"
#include <SDL2/SDL.h>
#include <vector>

m2::expected<m2::thirdparty::video::Renderer> m2::thirdparty::video::Renderer::Create(void* sdlWindow, const bool graphicsPixelated) {
	if (graphicsPixelated) {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	} else {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	}

	auto* rawRenderer = SDL_CreateRenderer(static_cast<SDL_Window*>(sdlWindow), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	if (not rawRenderer) {
		return make_unexpected(std::string{SDL_GetError()});
	}
	return Renderer{rawRenderer};
}

m2::thirdparty::video::Renderer::Renderer(Renderer&& other) noexcept : _renderer(other._renderer) {
	other._renderer = nullptr;
}
m2::thirdparty::video::Renderer& m2::thirdparty::video::Renderer::operator=(Renderer&& other) noexcept {
	std::swap(_renderer, other._renderer);
	return *this;
}
m2::thirdparty::video::Renderer::~Renderer() {
	if (_renderer) {
		SDL_DestroyRenderer(static_cast<SDL_Renderer*>(_renderer));
		_renderer = nullptr;
	}
}

m2::VecI m2::thirdparty::video::Renderer::GetOutputSize() const {
	int w, h;
	SDL_GetRendererOutputSize(static_cast<SDL_Renderer*>(_renderer), &w, &h);
	return {w, h};
}
std::string m2::thirdparty::video::Renderer::GetName() const {
	SDL_RendererInfo info;
	SDL_GetRendererInfo(static_cast<SDL_Renderer*>(_renderer), &info);
	return info.name;
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

void m2::thirdparty::video::Renderer::DrawLineStrip(const std::span<const VecF> pointsPx, const RGBA& color) {
	auto* sdlRenderer = static_cast<SDL_Renderer*>(_renderer);
	if (SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a) != 0) {
		throw M2_ERROR(std::string{"SDL_SetRenderDrawColor failed: "} + SDL_GetError());
	}
	std::vector<SDL_FPoint> sdlPoints;
	sdlPoints.reserve(pointsPx.size());
	for (const auto& point : pointsPx) {
		sdlPoints.push_back(ToSdlFPoint(point));
	}
	if (SDL_RenderDrawLinesF(sdlRenderer, sdlPoints.data(), I(sdlPoints.size())) != 0) {
		throw M2_ERROR(std::string{"SDL_RenderDrawLinesF failed: "} + SDL_GetError());
	}
}
