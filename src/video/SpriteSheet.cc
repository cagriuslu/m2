#include <m2/video/SpriteSheet.h>
#include <m2/Game.h>
#include <SDL2/SDL_image.h>

m2::SpriteSheet::SpriteSheet(const pb::SpriteSheet& spriteSheet, SDL_Renderer* renderer, bool lightning) : _pb(spriteSheet) {
	_surface.reset(IMG_Load((GetResourceDir() / spriteSheet.resource()).string().c_str()));
	if (not _surface) {
		throw M2_ERROR("SDL Error while loading " + spriteSheet.resource() + ": " + IMG_GetError());
	}
	_texture.reset(SDL_CreateTextureFromSurface(renderer, _surface.get()));
	if (not _texture) {
		throw M2_ERROR("SDL Error while creating texture from surface" + spriteSheet.resource() + ": " + IMG_GetError());
	}
	SDL_SetTextureBlendMode(_texture.get(), SDL_BLENDMODE_BLEND);
	if (lightning) {
		// Darken the texture
		SDL_SetTextureColorMod(_texture.get(), 127, 127, 127);
	}
}

std::vector<m2::SpriteSheet> m2::SpriteSheet::LoadSpriteSheets(const pb::SpriteSheets& spriteSheets, SDL_Renderer* renderer, bool lightning) {
	std::vector<SpriteSheet> sheets_vector;
	std::ranges::for_each(spriteSheets.sheets(), [&](const auto& sheet) {
		sheets_vector.emplace_back(SpriteSheet{sheet, renderer, lightning});
	});
	return sheets_vector;
}
