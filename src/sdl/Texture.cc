#include <m2/Game.h>
#include <m2/sdl/Texture.h>

m2::VecI m2::sdl::texture_dimensions(const TextureUniquePtr& texture) { return texture_dimensions(texture.get()); }

m2::VecI m2::sdl::texture_dimensions(SDL_Texture* texture) {
	int text_w = 0, text_h = 0;
	SDL_QueryTexture(texture, nullptr, nullptr, &text_w, &text_h);
	return {text_w, text_h};
}

void m2::sdl::render_texture_with_color_mod(const TextureUniquePtr& texture, const RectI& destination, const RGB& color_mod) {
	return render_texture_with_color_mod(texture.get(), destination, color_mod);
}

void m2::sdl::render_texture_with_color_mod(SDL_Texture* texture, const RectI& destination, const RGB& color_mod) {
	// Color modulate the texture
	SDL_SetTextureColorMod(texture, color_mod.r, color_mod.g, color_mod.b);
	// Set blend mode otherwise color may not blend with the background
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);
	// Draw texture
	auto sdl_rect = static_cast<SDL_Rect>(destination);
	SDL_RenderCopy(M2_GAME.renderer, texture, nullptr, &sdl_rect);
}
