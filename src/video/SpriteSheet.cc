#include <m2/video/SpriteSheet.h>
#include <m2/Game.h>

m2::SpriteSheet::SpriteSheet(const pb::SpriteSheet& spriteSheet, thirdparty::video::Renderer& renderer)
		: _pb(spriteSheet), _surface(thirdparty::video::Surface::CreateFromImageFile(GetResourceDir() / spriteSheet.resource())) {
	_texture = thirdparty::video::Texture::CreateFromSurface(renderer, _surface.RawHandle());
}

std::vector<m2::SpriteSheet> m2::SpriteSheet::LoadSpriteSheets(const pb::SpriteSheets& spriteSheets, thirdparty::video::Renderer& renderer) {
	std::vector<SpriteSheet> sheets_vector;
	std::ranges::for_each(spriteSheets.sheets(), [&](const auto& sheet) {
		sheets_vector.emplace_back(SpriteSheet{sheet, renderer});
	});
	return sheets_vector;
}
