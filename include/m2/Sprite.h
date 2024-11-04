#pragma once
#include <SDL2/SDL.h>
#include <Sprite.pb.h>
#include <m2g_ObjectType.pb.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "DynamicSheet.h"
#include "RectI.h"
#include "VecF.h"
#include "box2d/Body.h"
#include "protobuf/Detail.h"
#include "sdl/Detail.h"
#include "sdl/FontTexture.h"
#include <variant>

namespace m2 {
	struct ForegroundCompanion {};
	using DrawVariant = std::variant<std::monostate, ForegroundCompanion, pb::SpriteEffectType>; // monostate = default variant

	class SpriteSheet final {
		pb::SpriteSheet _sprite_sheet;
		std::unique_ptr<SDL_Surface, sdl::SurfaceDeleter> _surface;
		std::unique_ptr<SDL_Texture, sdl::TextureDeleter> _texture;

	   public:
		SpriteSheet(const pb::SpriteSheet& sprite_sheet, SDL_Renderer* renderer, bool lightning);
		[[nodiscard]] const pb::SpriteSheet& sprite_sheet() const;
		[[nodiscard]] SDL_Surface* surface() const;
		[[nodiscard]] SDL_Texture* texture() const;
	};

	class SpriteEffectsSheet : private DynamicSheet {
	   public:
		explicit SpriteEffectsSheet(SDL_Renderer* renderer);
		using DynamicSheet::texture;
		RectI create_mask_effect(
		    const SpriteSheet& sheet, const pb::RectI& rect, const pb::Color& mask_color, bool lightning);
		RectI create_foreground_companion_effect(
		    const SpriteSheet& sheet, const pb::RectI& rect,
		    const google::protobuf::RepeatedPtrField<pb::RectI>& rect_pieces, bool lightning);
		RectI create_grayscale_effect(const SpriteSheet& sheet, const pb::RectI& rect, bool lightning);
		RectI create_image_adjustment_effect(
		    const SpriteSheet& sheet, const pb::RectI& rect, const pb::ImageAdjustment& image_adjustment,
		    bool lightning);

		[[nodiscard]] inline int texture_width() const { return width(); }
		[[nodiscard]] inline int texture_height() const { return height(); }
	};

	class Sprite final {
		const SpriteSheet* _sprite_sheet{};
		const SpriteEffectsSheet* _effects_sheet{};

		std::optional<m2g::pb::SpriteType> _original_type;
		std::vector<RectI> _effects;
		std::optional<RectI> _foreground_companion_sprite_effects_sheet_rect;
		std::optional<VecF> _foreground_companion_center_to_origin_vec_px;
		std::optional<VecF> _foreground_companion_center_to_origin_vec_m;
		RectI _rect;
		float _original_rotation_radians{};
		int _ppm{};
		VecF _center_to_origin_vec_px;
		VecF _center_to_origin_vec_m;
		box2d::ColliderType _background_collider_type{box2d::ColliderType::NONE};
		VecF _background_collider_origin_to_origin_vec_m;
		VecF _background_collider_rect_dims_m;
		float _background_collider_circ_radius_m{};
		box2d::ColliderType _foreground_collider_type{box2d::ColliderType::NONE};
		VecF _foreground_collider_origin_to_origin_vec_m;
		VecF _foreground_collider_rect_dims_m;
		float _foreground_collider_circ_radius_m{};
		bool _is_background_tile{};
		std::vector<m2g::pb::ItemType> _named_items;

		// Text label
		std::optional<sdl::FontTexture> _font_texture;

	   public:
		Sprite() = default;
		Sprite(
		    const std::vector<SpriteSheet>& sprite_sheets, const SpriteSheet& sprite_sheet,
		    SpriteEffectsSheet& sprite_effects_sheet, const pb::Sprite& sprite, bool lightning);
		Sprite(SDL_Renderer* renderer, TTF_Font* font, int font_size, const pb::TextLabel& text_label);

		// Accessors
		[[nodiscard]] const SpriteSheet& sprite_sheet() const { return *_sprite_sheet; }
		[[nodiscard]] inline const SpriteEffectsSheet* effects_sheet() const { return _effects_sheet; }
		[[nodiscard]] std::optional<m2g::pb::SpriteType> original_type() const { return _original_type; }
		[[nodiscard]] SDL_Texture* effects_texture() const {
			return _effects_sheet ? _effects_sheet->texture() : nullptr;
		}
		[[nodiscard]] const RectI& effect_rect(pb::SpriteEffectType effect_type) const {
			return _effects[pb::enum_index(effect_type)];
		}
		[[nodiscard]] bool has_foreground_companion() const {
			return _foreground_companion_center_to_origin_vec_m.has_value();
		}
		[[nodiscard]] VecF foreground_companion_center_to_origin_vec_px() const {
			return _foreground_companion_center_to_origin_vec_px.value();
		}
		[[nodiscard]] VecF foreground_companion_center_to_origin_vec_m() const {
			return _foreground_companion_center_to_origin_vec_m.value();
		}
		[[nodiscard]] inline const RectI& rect() const { return _rect; }
		[[nodiscard]] float original_rotation_radians() const { return _original_rotation_radians; }
		[[nodiscard]] int ppm() const { return _ppm; }
		[[nodiscard]] inline const VecF& center_to_origin_vec_px() const { return _center_to_origin_vec_px; }
		[[nodiscard]] inline const VecF& center_to_origin_vec_m() const { return _center_to_origin_vec_m; }
		[[nodiscard]] inline box2d::ColliderType background_collider_type() const { return _background_collider_type; }
		[[nodiscard]] VecF background_collider_origin_to_origin_vec_m() const {
			return _background_collider_origin_to_origin_vec_m;
		}
		[[nodiscard]] VecF background_collider_rect_dims_m() const { return _background_collider_rect_dims_m; }
		[[nodiscard]] float background_collider_circ_radius_m() const { return _background_collider_circ_radius_m; }
		[[nodiscard]] inline box2d::ColliderType foreground_collider_type() const { return _foreground_collider_type; }
		[[nodiscard]] VecF foreground_collider_origin_to_origin_vec_m() const {
			return _foreground_collider_origin_to_origin_vec_m;
		}
		[[nodiscard]] VecF foreground_collider_rect_dims_m() const { return _foreground_collider_rect_dims_m; }
		[[nodiscard]] float foreground_collider_circ_radius_m() const { return _foreground_collider_circ_radius_m; }
		[[nodiscard]] inline bool is_background_tile() const { return _is_background_tile; }
		[[nodiscard]] const std::vector<m2g::pb::ItemType>& named_items() const { return _named_items; }

		[[nodiscard]] SDL_Texture* texture(DrawVariant draw_variant = {}) const;
		[[nodiscard]] VecF texture_total_dimensions(DrawVariant draw_variant) const;
		[[nodiscard]] const RectI& rect(DrawVariant draw_variant) const;

		/// Ratio of screen pixels to sprite pixels
		/// Multiply sprite dimensions (srcpx) with this number to convert them to screen dimensions (dstpx).
		[[nodiscard]] float sheet_to_screen_pixel_multiplier() const;

		/// Returns a vector from the sprite's center pixel to the sprite's origin.
		[[nodiscard]] VecF center_to_origin_srcpx(DrawVariant draw_variant) const;

		/// Returns a vector from the sprite's center pixel to the sprite's graphical origin in screen dimensions
		/// (dstpx).
		[[nodiscard]] inline VecF center_to_origin_dstpx(DrawVariant draw_variant) const {
			// Convert from source pixels to destination pixels
			return center_to_origin_srcpx(draw_variant) * sheet_to_screen_pixel_multiplier();
		}
	};

	std::vector<SpriteSheet> load_sprite_sheets(
	    const pb::SpriteSheets& sprite_sheets, SDL_Renderer* renderer, bool lightning);
	std::vector<Sprite> load_sprites(
	    const std::vector<SpriteSheet>& sprite_sheets,
	    const ::google::protobuf::RepeatedPtrField<pb::TextLabel>& text_labels,
	    SpriteEffectsSheet& sprite_effects_sheet, SDL_Renderer* renderer, TTF_Font* font, int font_size, bool lightning);
	std::vector<m2g::pb::SpriteType> list_level_editor_background_sprites(const std::vector<Sprite>& sprites);
	std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> list_level_editor_object_sprites(
	    const std::filesystem::path& objects_path);
}  // namespace m2
