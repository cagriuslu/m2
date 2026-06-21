#include <m2/Game.h>
#include <m2/sdl/Texture.h>

m2::VecI m2::sdl::texture_dimensions(const TextureUniquePtr& texture) { return texture_dimensions(texture.get()); }

m2::VecI m2::sdl::texture_dimensions(SDL_Texture* texture) {
	int text_w = 0, text_h = 0;
	SDL_QueryTexture(texture, nullptr, nullptr, &text_w, &text_h);
	return {text_w, text_h};
}

