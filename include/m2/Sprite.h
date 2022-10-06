#ifndef M2_SPRITE_H
#define M2_SPRITE_H

#include <Sprite.pb.h>
#include <ObjectType.pb.h>
#include "sdl/Utils.hh"
#include "Vec2f.h"
#include <SDL.h>
#include <string>
#include <memory>

namespace m2 {
	class SpriteSheet final {
		pb::SpriteSheet _sprite_sheet;
		std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> _surface;
		std::unique_ptr<SDL_Texture, SdlTextureDeleter> _texture;

	public:
		SpriteSheet(const pb::SpriteSheet& sprite_sheet, SDL_Renderer* renderer);
		[[nodiscard]] const pb::SpriteSheet& sprite_sheet() const;
		[[nodiscard]] SDL_Surface* surface() const;
		[[nodiscard]] SDL_Texture* texture() const;
	};

	class SpriteEffectsSheet final {
		SDL_Renderer* _renderer; // TODO Use weak-ptr instead
		std::unique_ptr<SDL_Surface, SdlSurfaceDeleter> _surface;
		int _h{};
		std::unique_ptr<SDL_Texture, SdlTextureDeleter> _texture;

	public:
		SpriteEffectsSheet(SDL_Renderer* renderer, uint32_t window_pixel_format);
		[[nodiscard]] SDL_Texture* texture() const;
		SDL_Rect create_effect(const SpriteSheet& sheet, const pb::Rect2i& rect, const pb::SpriteEffect& effect);
	};

	class Sprite final {
		const SpriteSheet* _sprite_sheet{};
		pb::Sprite _sprite;

		const SpriteEffectsSheet* _effects_sheet{};
		std::vector<SDL_Rect> _effects;

		unsigned _ppm{};
		Vec2f _center_offset_m;
		Vec2f _collider_center_offset_m;
		Vec2f _collider_rect_dims_m;
		float _collider_circ_radius_m{};

	public:
		Sprite() = default;
		Sprite(const SpriteSheet& sprite_sheet, SpriteEffectsSheet& sprite_effects_sheet, const pb::Sprite& sprite);
		[[nodiscard]] const SpriteSheet& sprite_sheet() const;
		[[nodiscard]] const pb::Sprite& sprite() const;
		[[nodiscard]] SDL_Texture* effects_texture() const;
		[[nodiscard]] SDL_Rect effect_rect(pb::SpriteEffectType effect_type) const;
		[[nodiscard]] unsigned ppm() const;
		[[nodiscard]] Vec2f center_offset_m() const;
		[[nodiscard]] Vec2f collider_center_offset_m() const;
		[[nodiscard]] Vec2f collider_rect_dims_m() const;
		[[nodiscard]] float collider_circ_radius_m() const;
	};

	std::vector<SpriteSheet> load_sprite_sheets(const std::string& sprite_sheets_path, SDL_Renderer* renderer);
	std::vector<Sprite> load_sprites(const std::vector<SpriteSheet>& sprite_sheets, SpriteEffectsSheet& sprite_effects_sheet);
	std::vector<m2g::pb::SpriteType> list_editor_background_sprites(const std::vector<SpriteSheet>& sprite_sheets);
	std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> list_editor_object_sprites(const std::string& objects_path);
}

#endif //M2_SPRITE_H
