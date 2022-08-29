#ifndef M2_SPRITE_H
#define M2_SPRITE_H

#include "component/Graphic.h"
#include <SpriteSheets.pb.h>
#include <SDL.h>
#include <string>

namespace m2 {
	class SpriteSheet final {
		pb::SpriteSheet _sprite_sheet;
		SDL_Texture *_texture;

	public:
		SpriteSheet(const pb::SpriteSheet& sprite_sheet, SDL_Renderer* renderer);
		~SpriteSheet();
		[[nodiscard]] const pb::SpriteSheet& sprite_sheet() const;
		[[nodiscard]] SDL_Texture* texture() const;
	};

	class Sprite {
		const SpriteSheet& _sprite_sheet;
		const pb::Sprite& _sprite;
		const comp::Graphic _example_gfx;

	public:
		Sprite(const SpriteSheet& sprite_sheet, const pb::Sprite& sprite);
		[[nodiscard]] const SpriteSheet& sprite_sheet() const;
		[[nodiscard]] const pb::Sprite& sprite() const;
		[[nodiscard]] comp::Graphic example_gfx() const;
	};

	using Sheets = std::unordered_map<std::string, SpriteSheet>;
	using Sprites = std::unordered_map<std::string, Sprite>;
	using SheetsAndSprites = std::pair<Sheets,Sprites>;
	SheetsAndSprites load_sheets_and_sprites(const std::string& sprite_sheets_path, SDL_Renderer* renderer);
}

#endif //M2_SPRITE_H
