#ifndef M2_SPRITE_H
#define M2_SPRITE_H

#include <Sprite.pb.h>
#include <ObjectType.pb.h>
#include "SdlUtils.hh"
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
		const SpriteSheet* _sprite_sheet{};
		pb::Sprite _sprite;
		unsigned _ppm{};
		Vec2f _center_offset_m;
		Vec2f _collider_center_offset_m;
		Vec2f _collider_rect_dims_m;
		float _collider_circ_radius_m{};

	public:
		Sprite() = default;
		Sprite(const SpriteSheet& sprite_sheet, const pb::Sprite& sprite);
		[[nodiscard]] const SpriteSheet& sprite_sheet() const;
		[[nodiscard]] const pb::Sprite& sprite() const;
		[[nodiscard]] unsigned ppm() const;
		[[nodiscard]] Vec2f center_offset_m() const;
		[[nodiscard]] Vec2f collider_center_offset_m() const;
		[[nodiscard]] Vec2f collider_rect_dims_m() const;
		[[nodiscard]] float collider_circ_radius_m() const;
	};

	std::vector<SpriteSheet> load_sprite_sheets(const std::string& sprite_sheets_path, SDL_Renderer* renderer);
	std::vector<Sprite> load_sprites(const std::vector<SpriteSheet>& sprite_sheets);
	std::vector<m2g::pb::SpriteType> list_editor_background_sprites(const std::vector<SpriteSheet>& sprite_sheets);
	std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> list_editor_object_sprites(const std::string& objects_path);
}

#endif //M2_SPRITE_H
