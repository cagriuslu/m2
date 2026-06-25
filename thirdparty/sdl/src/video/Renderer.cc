#include <m2/thirdparty/video/Renderer.h>
#include "SdlConversions.h"
#include <SDL3/SDL.h>
#include <vector>
#include <format>

m2::expected<m2::thirdparty::video::Renderer> m2::thirdparty::video::Renderer::Create(void* sdlWindow) {
	auto* rawRenderer = SDL_CreateRenderer(static_cast<SDL_Window*>(sdlWindow), nullptr);
	if (not rawRenderer) {
		return make_unexpected(std::format("SDL_CreateRenderer error: {}", SDL_GetError()));
	}
	SDL_SetRenderDrawBlendMode(rawRenderer, SDL_BLENDMODE_BLEND);
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
	SDL_GetCurrentRenderOutputSize(static_cast<SDL_Renderer*>(_renderer), &w, &h);
	return {w, h};
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

void m2::thirdparty::video::Renderer::DrawLineStrip(const std::span<const VecF> pointsPx, const RGBA& color) {
	auto* sdlRenderer = static_cast<SDL_Renderer*>(_renderer);
	if (not SDL_SetRenderDrawColor(sdlRenderer, color.r, color.g, color.b, color.a)) {
		throw M2_ERROR(std::string{"SDL_SetRenderDrawColor error: "} + SDL_GetError());
	}
	std::vector<SDL_FPoint> sdlPoints;
	sdlPoints.reserve(pointsPx.size());
	for (const auto& point : pointsPx) {
		sdlPoints.push_back(ToSdlFPoint(point));
	}
	if (not SDL_RenderLines(sdlRenderer, sdlPoints.data(), I(sdlPoints.size()))) {
		throw M2_ERROR(std::string{"SDL_RenderLines error: "} + SDL_GetError());
	}
}
