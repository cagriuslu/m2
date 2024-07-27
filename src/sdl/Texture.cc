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
