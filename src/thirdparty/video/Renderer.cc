#include <m2/thirdparty/video/Renderer.h>
#include <m2/Log.h>
#include <SDL2/SDL.h>

m2::expected<m2::thirdparty::video::Renderer> m2::thirdparty::video::Renderer::Create(void* sdlWindow, const bool graphicsPixelated) {
	if (graphicsPixelated) {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	} else {
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	}

	auto* rawRenderer = SDL_CreateRenderer(static_cast<SDL_Window*>(sdlWindow), -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	if (not rawRenderer) {
		return make_unexpected(std::string{SDL_GetError()});
	}

	SDL_RendererInfo info;
	SDL_GetRendererInfo(rawRenderer, &info);
	LOG_INFO("Renderer", info.name);

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
