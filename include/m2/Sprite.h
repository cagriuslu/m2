#ifndef M2_SPRITE_H
#define M2_SPRITE_H

#include "component/Graphic.h"
#include <SpriteSheets.pb.h>
#include "SDLUtils.hh"
#include <SDL.h>
#include <string>
#include <memory>

namespace m2 {
	class SpriteSheet final {
		pb::SpriteSheet _sprite_sheet;
		std::unique_ptr<SDL_Texture, m2::SdlTextureDeleter> _texture;

	public:
		SpriteSheet(const pb::SpriteSheet& sprite_sheet, SDL_Renderer* renderer);
		[[nodiscard]] const pb::SpriteSheet& sprite_sheet() const;
		[[nodiscard]] SDL_Texture* texture() const;
	};

	class Sprite {
		const SpriteSheet& _sprite_sheet;
		pb::Sprite _sprite;
		const unsigned _ppm;

	public:
		Sprite(const SpriteSheet& sprite_sheet, const pb::Sprite& sprite);
		[[nodiscard]] const SpriteSheet& sprite_sheet() const;
		[[nodiscard]] const pb::Sprite& sprite() const;
		[[nodiscard]] unsigned ppm() const;
	};

	using Sheets = std::unordered_map<std::string, SpriteSheet>;
	using Sprites = std::unordered_map<std::string, Sprite>;
	using SheetsAndSprites = std::pair<Sheets,Sprites>;
	SheetsAndSprites load_sheets_and_sprites(const std::string& sprite_sheets_path, SDL_Renderer* renderer);
}

#endif //M2_SPRITE_H
