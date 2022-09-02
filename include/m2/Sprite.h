#ifndef M2_SPRITE_H
#define M2_SPRITE_H

#include <SpriteSheets.pb.h>
#include "SDLUtils.hh"
#include "Vec2f.h"
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
		std::string _key;
		const unsigned _ppm;
		const Vec2f _center_offset_m;
		const Vec2f _collider_center_offset_m;
		const Vec2f _collider_rect_dims_m;
		const float _collider_circ_radius_m;

	public:
		Sprite(const SpriteSheet& sprite_sheet, const pb::Sprite& sprite, const std::string& key);
		[[nodiscard]] const SpriteSheet& sprite_sheet() const;
		[[nodiscard]] const pb::Sprite& sprite() const;
		[[nodiscard]] const std::string& key() const;
		[[nodiscard]] unsigned ppm() const;
		[[nodiscard]] Vec2f center_offset_m() const;
		[[nodiscard]] Vec2f collider_center_offset_m() const;
		[[nodiscard]] Vec2f collider_rect_dims_m() const;
		[[nodiscard]] float collider_circ_radius_m() const;
	};

	using Sheets = std::unordered_map<std::string, SpriteSheet>;
	using Sprites = std::unordered_map<std::string, Sprite>;
	using SheetsAndSprites = std::pair<Sheets,Sprites>;
	SheetsAndSprites load_sheets_and_sprites(const std::string& sprite_sheets_path, SDL_Renderer* renderer);

	using SpriteLut = std::vector<const Sprite*>;
	SpriteLut generate_sprite_lut(const Sprites& sprites_map);
}

#endif //M2_SPRITE_H
