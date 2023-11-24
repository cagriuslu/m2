#pragma once
#include <Sprite.pb.h>
#include "DynamicSheet.h"
#include <m2g_ObjectType.pb.h>
#include "sdl/Detail.h"
#include "VecF.h"
#include <SDL2/SDL.h>
#include <optional>
#include <filesystem>
#include <string>
#include <memory>
#include "RectI.h"
#include "box2d/Body.h"

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
		RectI create_mask_effect(const SpriteSheet& sheet, const pb::RectI& rect, const pb::Color& mask_color);
		RectI create_foreground_companion_effect(const SpriteSheet& sheet, const pb::RectI &rect, const google::protobuf::RepeatedPtrField<pb::RectI>& rect_pieces);
		RectI create_grayscale_effect(const SpriteSheet& sheet, const pb::RectI &rect);
		RectI create_image_adjustment_effect(const SpriteSheet& sheet, const pb::RectI &rect, const pb::ImageAdjustment& image_adjustment);

		[[nodiscard]] inline int texture_width() const { return width(); }
		[[nodiscard]] inline int texture_height() const { return height(); }
	};

	class Sprite final {
		const SpriteSheet* _sprite_sheet{};
		pb::Sprite _sprite;

		const SpriteEffectsSheet* _effects_sheet{};
		std::vector<RectI> _effects;
		std::optional<VecF> _foreground_companion_center_offset_px{};
		std::optional<VecF> _foreground_companion_center_offset_m{};
		RectI _rect;
		float _original_rotation_radians{};
		int _ppm{};
		VecF _center_offset_px;
		VecF _center_offset_m;
		box2d::ColliderType _background_collider_type{box2d::ColliderType::NONE};
		VecF _background_collider_center_offset_m;
		VecF _background_collider_rect_dims_m;
		float _background_collider_circ_radius_m{};
		box2d::ColliderType _foreground_collider_type{box2d::ColliderType::NONE};
		VecF _foreground_collider_center_offset_m;
		VecF _foreground_collider_rect_dims_m;
		float _foreground_collider_circ_radius_m{};
		bool _is_background_tile{};

	public:
		Sprite() = default;
		Sprite(const SpriteSheet& sprite_sheet, SpriteEffectsSheet& sprite_effects_sheet, const pb::Sprite& sprite);

		// Accessors
		[[nodiscard]] const SpriteSheet& sprite_sheet() const;
		[[nodiscard]] inline const SpriteEffectsSheet* effects_sheet() const { return _effects_sheet; }
		[[nodiscard]] SDL_Texture* effects_texture() const;
		[[nodiscard]] RectI effect_rect(pb::SpriteEffectType effect_type) const;
		[[nodiscard]] bool has_foreground_companion() const;
		[[nodiscard]] VecF foreground_companion_center_offset_px() const;
		[[nodiscard]] VecF foreground_companion_center_offset_m() const;
		[[nodiscard]] inline const RectI& rect() const { return _rect; }
		[[nodiscard]] float original_rotation_radians() const;
		[[nodiscard]] int ppm() const;
		[[nodiscard]] inline const VecF& center_offset_px() const { return _center_offset_px; }
		[[nodiscard]] inline const VecF& center_offset_m() const { return _center_offset_m; }
		[[nodiscard]] inline box2d::ColliderType background_collider_type() const { return _background_collider_type; }
		[[nodiscard]] VecF background_collider_center_offset_m() const;
		[[nodiscard]] VecF background_collider_rect_dims_m() const;
		[[nodiscard]] float background_collider_circ_radius_m() const;
		[[nodiscard]] inline box2d::ColliderType foreground_collider_type() const { return _foreground_collider_type; }
		[[nodiscard]] VecF foreground_collider_center_offset_m() const;
		[[nodiscard]] VecF foreground_collider_rect_dims_m() const;
		[[nodiscard]] float foreground_collider_circ_radius_m() const;
		[[nodiscard]] inline bool is_background_tile() const { return _is_background_tile; }

		/// Ratio of screen pixels to sprite pixels
		/// Multiply sprite dimensions (srcpx) with this number to convert them to screen dimensions (dstpx).
		[[nodiscard]] float sheet_to_screen_pixel_multiplier() const;

		/// Returns a vector from the sprite's center pixel to the sprite's origin.
		[[nodiscard]] VecF center_to_origin_srcpx(pb::SpriteEffectType effect_type) const;

		/// Returns a vector from the sprite's center pixel to the sprite's graphical origin in screen dimensions (dstpx).
		[[nodiscard]] inline VecF center_to_origin_dstpx(pb::SpriteEffectType effect_type) const {
			// Convert from source pixels to destination pixels
			return center_to_origin_srcpx(effect_type) * sheet_to_screen_pixel_multiplier();
		}
	};

	std::vector<SpriteSheet> load_sprite_sheets(const std::filesystem::path& sprite_sheets_path, SDL_Renderer* renderer);
	std::vector<Sprite> load_sprites(const std::vector<SpriteSheet>& sprite_sheets, SpriteEffectsSheet& sprite_effects_sheet);
	std::vector<m2g::pb::SpriteType> list_level_editor_background_sprites(const std::vector<SpriteSheet>& sprite_sheets);
	std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> list_level_editor_object_sprites(const std::filesystem::path& objects_path);
}
