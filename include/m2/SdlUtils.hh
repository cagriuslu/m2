#ifndef SDL_UTILS_H
#define SDL_UTILS_H

#include <Rect2i.pb.h>
#include <SDL.h>

SDL_Cursor* SdlUtils_CreateCursor();

uint32_t SdlUtils_GetTicksAtLeast1ms(uint32_t lastTicks, uint32_t nongame_ticks);

namespace m2 {
	namespace sdl {
		SDL_Rect expand_rect(const SDL_Rect& rect, int diff);
		SDL_Rect to_rect(const pb::Rect2i& pb_rect);
	}

	struct SdlTextureDeleter {
		void operator()(SDL_Texture* t);
	};
}

#endif
