#include <SDL2/SDL_ttf.h>
#include <m2/Game.h>
#include <m2/sdl/Font.h>

namespace {
	m2::expected<std::tuple<SDL_Texture*, int, int>> generate_font_texture(
	    TTF_Font* font, SDL_Renderer* renderer, const std::string& str, SDL_Color color) {
		// Render to surface
		SDL_Surface* surf = TTF_RenderUTF8_Blended(font, str.c_str(), color);
		if (!surf) {
			return m2::make_unexpected(TTF_GetError());
		}
		int w = surf->w;
		int h = surf->h;

		// Store previous render quality
		const char* prev_render_quality = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
		// Create texture with linear filtering
		SDL_SetHint(
		    SDL_HINT_RENDER_SCALE_QUALITY, "1");  // Linear filtering is less crisp, but more readable when small

		// Create texture
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surf);
		SDL_FreeSurface(surf);  // Free surface right away
		SDL_SetHint(
		    SDL_HINT_RENDER_SCALE_QUALITY, prev_render_quality);  // Reinstate previous render quality right away

		if (!texture) {
			return m2::make_unexpected(SDL_GetError());
		}
		return std::make_tuple(texture, w, h);
	}
}  // namespace

m2::expected<m2::sdl::FontTexture> m2::sdl::FontTexture::create(
    TTF_Font* font, SDL_Renderer* renderer, int number, SDL_Color color) {
	auto expect_texture = generate_font_texture(font, renderer, std::to_string(number), color);
	m2_reflect_unexpected(expect_texture);
	return FontTexture{
	    std::get<0>(*expect_texture), number, std::get<1>(*expect_texture), std::get<2>(*expect_texture)};
}

m2::expected<m2::sdl::FontTexture> m2::sdl::FontTexture::create(
    TTF_Font* font, SDL_Renderer* renderer, float number, SDL_Color color) {
	auto expect_texture = generate_font_texture(font, renderer, std::to_string(number), color);
	m2_reflect_unexpected(expect_texture);
	return FontTexture{
	    std::get<0>(*expect_texture), number, std::get<1>(*expect_texture), std::get<2>(*expect_texture)};
}

m2::expected<m2::sdl::FontTexture> m2::sdl::FontTexture::create(
    TTF_Font* font, SDL_Renderer* renderer, const std::string& text, SDL_Color color) {
	if (text.empty()) {
		return FontTexture{nullptr, text, 0, 0};
	}
	auto expect_texture = generate_font_texture(font, renderer, text, color);
	m2_reflect_unexpected(expect_texture);
	return FontTexture{std::get<0>(*expect_texture), text, std::get<1>(*expect_texture), std::get<2>(*expect_texture)};
}
