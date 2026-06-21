#include <m2/video/SpriteSheet.h>
#include <m2/Game.h>
#include <SDL2/SDL_image.h>

m2::SpriteSheet::SpriteSheet(const pb::SpriteSheet& spriteSheet, SDL_Renderer* renderer) : _pb(spriteSheet) {
	_surface.reset(IMG_Load((GetResourceDir() / spriteSheet.resource()).string().c_str()));
	if (not _surface) {
		throw M2_ERROR("SDL Error while loading " + spriteSheet.resource() + ": " + IMG_GetError());
	}
	_texture = thirdparty::video::Texture::CreateFromSurface(renderer, _surface.get());
	auto* rawTexture = static_cast<SDL_Texture*>(_texture->RawHandle());
	SDL_SetTextureBlendMode(rawTexture, SDL_BLENDMODE_BLEND);
}

std::vector<m2::SpriteSheet> m2::SpriteSheet::LoadSpriteSheets(const pb::SpriteSheets& spriteSheets, SDL_Renderer* renderer) {
	std::vector<SpriteSheet> sheets_vector;
	std::ranges::for_each(spriteSheets.sheets(), [&](const auto& sheet) {
		sheets_vector.emplace_back(SpriteSheet{sheet, renderer});
	});
	return sheets_vector;
}
