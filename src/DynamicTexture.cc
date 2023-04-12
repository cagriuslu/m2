#include <m2/DynamicTexture.h>
#include <m2/Exception.h>

namespace {
	constexpr int default_texture_width = 512;
}

m2::DynamicTexture::DynamicTexture(SDL_Renderer *renderer) : _renderer(renderer) {
	auto* texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_TARGET, default_texture_width, default_texture_width);
	if (!texture) {
		throw M2FATAL("Unable to create texture: " + std::string{SDL_GetError()});
	}
	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND); // Allow transparent pixels
	// Clear the texture with transparent pixels
	SDL_SetRenderTarget(renderer, texture);
	SDL_SetRenderDrawColor(renderer, 0, 255, 0, 0);
	SDL_RenderClear(renderer);
	SDL_SetRenderTarget(renderer, nullptr);
	_texture.reset(texture);
}

SDL_Renderer* m2::DynamicTexture::renderer() const {
	return _renderer;
}

SDL_Texture* m2::DynamicTexture::texture() const {
	return _texture.get();
}

SDL_Rect m2::DynamicTexture::alloc(int w, int h) {
	// Check if the width is enough
	if (default_texture_width < w) {
		throw M2ERROR("DynamicTexture width would exceed");
	}

	// Resize texture if necessary
	int texture_height;
	SDL_QueryTexture(_texture.get(), nullptr, nullptr, nullptr, &texture_height);
	if (texture_height < _h + h) {
		auto* new_texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_TARGET, default_texture_width, texture_height * 3 / 2);
		if (!new_texture) {
			throw M2FATAL("Unable to recreate texture: " + std::string{SDL_GetError()});
		}
		if (SDL_SetRenderTarget(_renderer, new_texture)) {
			throw M2FATAL("Unable to set render target: " + std::string{SDL_GetError()});
		}
		SDL_Rect dest_rect{0, 0, default_texture_width, texture_height};
		if (SDL_RenderCopy(_renderer, _texture.get(), nullptr, &dest_rect)) {
			throw M2FATAL("Unable to copy texture: " + std::string{SDL_GetError()});
		}
		_texture.reset(new_texture);
	}

	SDL_Rect ret{0, _h, w, h};
	_h += h;
	return ret;
}
