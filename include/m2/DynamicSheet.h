#pragma once
#include "sdl/Detail.hh"
#include "RectI.h"
#include <SDL2/SDL.h>
#include <memory>

namespace m2 {
	/// Sprite sheet that dynamically increases in size on demand
	class DynamicSheet {
		SDL_Renderer* _renderer; // TODO Use weak-ptr instead
		std::unique_ptr<SDL_Surface, sdl::SurfaceDeleter> _surface;
		int _h{};
		std::unique_ptr<SDL_Texture, sdl::TextureDeleter> _texture;

	public:
		explicit DynamicSheet(SDL_Renderer* renderer);
		[[nodiscard]] SDL_Texture* texture() const;
		std::pair<SDL_Surface*, RectI> alloc(int w, int h);
		SDL_Texture* recreate_texture();
	protected:
		[[nodiscard]] inline int width() const { return _surface->w; }
		[[nodiscard]] inline int height() const { return _surface->h; }
	};
}
