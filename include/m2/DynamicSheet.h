#ifndef M2_DYNAMICSHEET_H
#define M2_DYNAMICSHEET_H

#include "sdl/Utils.hh"
#include <SDL2/SDL.h>
#include <memory>

namespace m2 {
	class DynamicSheet {
		SDL_Renderer* _renderer; // TODO Use weak-ptr instead
		std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> _surface;
		int _h{};
		std::unique_ptr<SDL_Texture, SdlTextureDeleter> _texture;

	public:
		explicit DynamicSheet(SDL_Renderer* renderer);
		[[nodiscard]] SDL_Texture* texture() const;
		std::pair<SDL_Surface*, SDL_Rect> alloc(int w, int h);
		SDL_Texture* recreate_texture();
	};
}

#endif //M2_DYNAMICSHEET_H
