#ifndef M2_SPRITE_H
#define M2_SPRITE_H

#include <Sprite.pb.h>
#include <m2g/SpriteId.h>
#include "SdlUtils.hh"
#include "Vec2f.h"
#include <SDL.h>
#include <string>
#include <memory>

namespace m2 {
	using SpriteSheetName = std::string;

	class SpriteSheet final {
		pb::SpriteSheet _sprite_sheet;
		std::unique_ptr<SDL_Texture, m2::SdlTextureDeleter> _texture;

	public:
		SpriteSheet(const pb::SpriteSheet& sprite_sheet, SDL_Renderer* renderer);
		[[nodiscard]] const pb::SpriteSheet& sprite_sheet() const;
		[[nodiscard]] SDL_Texture* texture() const;
	};

	using SpriteKey = std::string;

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
		Sprite(const SpriteSheet& sprite_sheet, const pb::Sprite& sprite, const SpriteKey& key);
		[[nodiscard]] const SpriteSheet& sprite_sheet() const;
		[[nodiscard]] const pb::Sprite& sprite() const;
		[[nodiscard]] const std::string& key() const;
		[[nodiscard]] unsigned ppm() const;
		[[nodiscard]] Vec2f center_offset_m() const;
		[[nodiscard]] Vec2f collider_center_offset_m() const;
		[[nodiscard]] Vec2f collider_rect_dims_m() const;
		[[nodiscard]] float collider_circ_radius_m() const;
	};

	using SpriteSheetKeyToSpriteSheetMap = std::unordered_map<SpriteSheetName, SpriteSheet>;
	using SpriteKeyToSpriteMap = std::unordered_map<SpriteKey, Sprite>;
	using SpriteMaps = std::pair<SpriteSheetKeyToSpriteSheetMap,SpriteKeyToSpriteMap>;
	SpriteMaps load_sprite_maps(const std::string& sprite_sheets_path, SDL_Renderer* renderer);

	using SpriteIdToSpriteLut = std::vector<const Sprite*>;
	using SpriteKeyToSpriteIdMap = std::unordered_map<SpriteKey, m2g::SpriteId>;
	using SpriteIdLuts = std::pair<SpriteIdToSpriteLut,SpriteKeyToSpriteIdMap>;
	SpriteIdLuts generate_sprite_id_luts(const SpriteKeyToSpriteMap& sprites_map);

	using EditorPaletteSpriteKeys = std::vector<SpriteKey>;
	using EditorPalettes = std::pair<EditorPaletteSpriteKeys,EditorPaletteSpriteKeys>;
	EditorPalettes generate_editor_palette_sprite_keys(const SpriteKeyToSpriteMap& sprites_map);
}

#endif //M2_SPRITE_H
