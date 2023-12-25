#include <m2/sdl/Texture.h>
#include <m2/Game.h>

m2::sdl::TextureUniquePtr m2::sdl::capture_screen_as_texture() {
    // Get screen size
    int w, h;
    SDL_GetRendererOutputSize(GAME.renderer, &w, &h);

    // Create surface
    auto *surface = SDL_CreateRGBSurface(0, w, h, 24, 0xFF, 0xFF00, 0xFF0000, 0);

    // Read pixels into surface
    SDL_RenderReadPixels(GAME.renderer, nullptr, SDL_PIXELFORMAT_RGB24, surface->pixels, surface->pitch);

    // Create texture
    TextureUniquePtr texture(SDL_CreateTextureFromSurface(GAME.renderer, surface));

    // Free surface
    SDL_FreeSurface(surface);

    return texture;
}
