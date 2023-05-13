#ifndef M2_SPRITE_H
#define M2_SPRITE_H

#include <Sprite.pb.h>
#include "Glyph.h"
#include "DynamicSheet.h"
#include <ObjectType.pb.h>
#include "sdl/Detail.hh"
#include "Vec2f.h"
#include <SDL2/SDL.h>
#include <optional>
#include <filesystem>
#include <string>
#include <memory>

namespace m2 {
	class SpriteSheet final {
		pb::SpriteSheet _sprite_sheet;
		std::unique_ptr<SDL_Surface, sdl::SurfaceDeleter> _surface;
		std::unique_ptr<SDL_Texture, sdl::TextureDeleter> _texture;

	public:
		SpriteSheet(const pb::SpriteSheet& sprite_sheet, SDL_Renderer* renderer);
		[[nodiscard]] const pb::SpriteSheet& sprite_sheet() const;
		[[nodiscard]] SDL_Surface* surface() const;
		[[nodiscard]] SDL_Texture* texture() const;
	};

	class SpriteEffectsSheet : private DynamicSheet {
	public:
		explicit SpriteEffectsSheet(SDL_Renderer* renderer);
		using DynamicSheet::texture;
		SDL_Rect create_mask_effect(const SpriteSheet& sheet, const pb::Rect2i& rect, const pb::Color& mask_color);
		SDL_Rect create_foreground_companion_effect(const SpriteSheet& sheet, const pb::Rect2i &rect, const google::protobuf::RepeatedPtrField<pb::Rect2i>& rect_pieces);
		SDL_Rect create_grayscale_effect(const SpriteSheet& sheet, const pb::Rect2i &rect);
	};

	class Sprite final {
		const SpriteSheet* _sprite_sheet{};
		pb::Sprite _sprite;

		const SpriteEffectsSheet* _effects_sheet{};
		std::vector<SDL_Rect> _effects;
		std::optional<Vec2f> _foreground_companion_center_offset_px{};
		std::optional<Vec2f> _foreground_companion_center_offset_m{};

		float _original_rotation_radians{};
		int _ppm{};
		Vec2f _background_collider_center_offset_m;
		Vec2f _background_collider_rect_dims_m;
		float _background_collider_circ_radius_m{};
		Vec2f _foreground_collider_center_offset_m;
		Vec2f _foreground_collider_rect_dims_m;
		float _foreground_collider_circ_radius_m{};

	public:
		Sprite() = default;
		Sprite(const SpriteSheet& sprite_sheet, SpriteEffectsSheet& sprite_effects_sheet, const pb::Sprite& sprite);
		[[nodiscard]] const SpriteSheet& sprite_sheet() const;
		[[nodiscard]] const pb::Sprite& sprite() const;
		[[nodiscard]] SDL_Texture* effects_texture() const;
		[[nodiscard]] SDL_Rect effect_rect(pb::SpriteEffectType effect_type) const;
		[[nodiscard]] bool has_foreground_companion() const;
		[[nodiscard]] Vec2f foreground_companion_center_offset_px() const;
		[[nodiscard]] Vec2f foreground_companion_center_offset_m() const;
		[[nodiscard]] float original_rotation_radians() const;
		[[nodiscard]] int ppm() const;
		[[nodiscard]] Vec2f background_collider_center_offset_m() const;
		[[nodiscard]] Vec2f background_collider_rect_dims_m() const;
		[[nodiscard]] float background_collider_circ_radius_m() const;
		[[nodiscard]] Vec2f foreground_collider_center_offset_m() const;
		[[nodiscard]] Vec2f foreground_collider_rect_dims_m() const;
		[[nodiscard]] float foreground_collider_circ_radius_m() const;
	};

	std::vector<SpriteSheet> load_sprite_sheets(const std::filesystem::path& sprite_sheets_path, SDL_Renderer* renderer);
	std::vector<Sprite> load_sprites(const std::vector<SpriteSheet>& sprite_sheets, SpriteEffectsSheet& sprite_effects_sheet);
	std::vector<m2g::pb::SpriteType> list_level_editor_background_sprites(const std::vector<SpriteSheet>& sprite_sheets);
	std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> list_level_editor_object_sprites(const std::filesystem::path& objects_path);
}

#endif //M2_SPRITE_H
