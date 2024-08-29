#include <m2/Game.h>
#include <m2/sdl/Texture.h>

m2::sdl::TextureUniquePtr m2::sdl::create_drawable_texture_of_screen_size() {
	// Get screen size
	int w, h;
	SDL_GetRendererOutputSize(M2_GAME.renderer, &w, &h);

	// Create texture
	return TextureUniquePtr{SDL_CreateTexture(M2_GAME.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w, h)};
}

m2::sdl::TextureUniquePtr m2::sdl::capture_screen_as_texture() {
	// Get screen size
	int w, h;
	SDL_GetRendererOutputSize(M2_GAME.renderer, &w, &h);

	// Create surface
	auto* surface = SDL_CreateRGBSurface(0, w, h, 24, 0xFF, 0xFF00, 0xFF0000, 0);

	// Read pixels into surface
	SDL_RenderReadPixels(M2_GAME.renderer, nullptr, SDL_PIXELFORMAT_RGB24, surface->pixels, surface->pitch);

	// Create texture
	TextureUniquePtr texture(SDL_CreateTextureFromSurface(M2_GAME.renderer, surface));

	// Free surface
	SDL_FreeSurface(surface);

	return texture;
}

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
