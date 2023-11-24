#pragma once
#include "sdl/Texture.h"

namespace m2 {
	/// Texture that dynamically increases in size on demand
	class DynamicTexture {
		SDL_Renderer* _renderer;
		std::unique_ptr<SDL_Texture, sdl::TextureDeleter> _texture;
		int _h{};

	public:
		explicit DynamicTexture(SDL_Renderer* renderer);
		[[nodiscard]] SDL_Renderer* renderer() const;
		[[nodiscard]] SDL_Texture* texture() const;
		SDL_Rect alloc(int w, int h);
	};
}
