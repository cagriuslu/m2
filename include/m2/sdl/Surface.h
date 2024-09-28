#pragma once
#include <SDL2/SDL.h>
#include <memory>

namespace m2::sdl {
    struct SurfaceDeleter {
        void operator()(SDL_Surface* s) const { SDL_FreeSurface(s); }
    };
    using SurfaceUniquePtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;
}
