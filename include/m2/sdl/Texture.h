#pragma once

#include <SDL2/SDL.h>

namespace m2::sdl {
    struct TextureDeleter {
        void operator()(SDL_Texture* t) const { SDL_DestroyTexture(t); }
    };
    using TextureUniquePtr = std::unique_ptr<SDL_Texture, TextureDeleter>;
}
