#ifndef M2_DYNAMICSHEET_H
#define M2_DYNAMICSHEET_H

#include "sdl/Utils.hh"
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
		std::pair<SDL_Surface*, SDL_Rect> alloc(int w, int h);
		SDL_Texture* recreate_texture();
	};
}

#endif //M2_DYNAMICSHEET_H
