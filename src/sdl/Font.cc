#include <m2/sdl/Font.h>
#include <m2/Game.h>
#include <SDL2/SDL_ttf.h>

m2::expected<m2::sdl::FontTexture> m2::sdl::FontTexture::create(const std::string& text, SDL_Color color) {
    // Render to surface
    SDL_Surface *surf = TTF_RenderUTF8_Blended(GAME.font, text.c_str(), color);
    if (!surf) {
        return m2::make_unexpected(TTF_GetError());
    }

    // Store previous render quality
    const char* prev_render_quality = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
    // Create texture with linear filtering
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Linear filtering is less crisp, but more readable when small

    // Create texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME.renderer, surf);
    SDL_FreeSurface(surf); // Free surface right away
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, prev_render_quality); // Reinstate previous render quality right away

    if (!texture) {
        return m2::make_unexpected(SDL_GetError());
    }
    return FontTexture{texture, text};
}

m2::sdl::TextureUniquePtr m2::sdl::generate_font(const std::string& text, SDL_Color color) {
    SDL_Surface *surf = TTF_RenderUTF8_Blended(GAME.font, text.c_str(), color);

    // Store previous render quality
    const char* prev_render_quality = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
    // Create texture with linear filtering
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"); // Linear filtering is less crisp, but more readable when small
    SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME.renderer, surf);
    // Reinstate previous render quality
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, prev_render_quality);

    SDL_FreeSurface(surf);
    return TextureUniquePtr{texture};
}
