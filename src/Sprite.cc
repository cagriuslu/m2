#include <Object.pb.h>
#include <SDL2/SDL_image.h>
#include <m2/Exception.h>
#include <m2/Game.h>
#include <m2/M2.h>
#include <m2/Proxy.h>
#include <m2/Sprite.h>
#include <m2/protobuf/Detail.h>

m2::SpriteSheet::SpriteSheet(const pb::SpriteSheet& sprite_sheet, SDL_Renderer* renderer, bool lightning)
    : _sprite_sheet(sprite_sheet) {
	_surface.reset(IMG_Load(sprite_sheet.resource().c_str()));
	if (not _surface) {
		throw M2ERROR("SDL Error while loading " + sprite_sheet.resource() + ": " + IMG_GetError());
	}
	_texture.reset(SDL_CreateTextureFromSurface(renderer, _surface.get()));
	if (not _texture) {
		throw M2ERROR(
		    "SDL Error while creating texture from surface" + sprite_sheet.resource() + ": " + IMG_GetError());
	}
	SDL_SetTextureBlendMode(_texture.get(), SDL_BLENDMODE_BLEND);
	if (lightning) {
		// Darken the texture
		SDL_SetTextureColorMod(_texture.get(), 127, 127, 127);
	}
}
const m2::pb::SpriteSheet& m2::SpriteSheet::sprite_sheet() const { return _sprite_sheet; }
SDL_Surface* m2::SpriteSheet::surface() const {
	return _surface.get();  // TODO potentially dangerous, use shared_ptr instead
}
SDL_Texture* m2::SpriteSheet::texture() const {
	return _texture.get();  // TODO potentially dangerous, use shared_ptr instead
}

m2::SpriteEffectsSheet::SpriteEffectsSheet(SDL_Renderer* renderer) : DynamicSheet(renderer) {}
m2::RectI m2::SpriteEffectsSheet::create_mask_effect(
    const SpriteSheet& sheet, const pb::RectI& rect, const pb::Color& mask_color, bool lightning) {
	auto [dst_surface, dst_rect] = alloc(rect.w(), rect.h());

	// Check pixel stride
	auto* src_surface = sheet.surface();
	if (src_surface->format->BytesPerPixel != 4 || dst_surface->format->BytesPerPixel != 4) {
		throw M2FATAL("Surface has unsupported pixel format");
	}

	// Prepare mask color
	uint32_t dst_color =
	    ((mask_color.r() >> dst_surface->format->Rloss) << dst_surface->format->Rshift) & dst_surface->format->Rmask;
	dst_color |=
	    ((mask_color.g() >> dst_surface->format->Gloss) << dst_surface->format->Gshift) & dst_surface->format->Gmask;
	dst_color |=
	    ((mask_color.b() >> dst_surface->format->Bloss) << dst_surface->format->Bshift) & dst_surface->format->Bmask;
	dst_color |=
	    ((mask_color.a() >> dst_surface->format->Aloss) << dst_surface->format->Ashift) & dst_surface->format->Amask;

	SDL_LockSurface(src_surface);
	SDL_LockSurface(dst_surface);

	for (int y = rect.y(); y < rect.y() + rect.h(); ++y) {
		for (int x = rect.x(); x < rect.x() + rect.w(); ++x) {
			// Read src pixel
			auto* src_pixels = static_cast<uint32_t*>(src_surface->pixels);
			auto src_pixel = *(src_pixels + (x + y * src_surface->w));

			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dst_surface->pixels);
			auto* dst_pixel = dst_pixels + ((x - rect.x()) + (y - rect.y() + dst_rect.y) * dst_surface->w);
			*dst_pixel = (src_pixel & src_surface->format->Amask) ? dst_color : 0;
		}
	}

	SDL_UnlockSurface(dst_surface);
	SDL_UnlockSurface(src_surface);

	recreate_texture(lightning);

	return dst_rect;
}
m2::RectI m2::SpriteEffectsSheet::create_foreground_companion_effect(
    const SpriteSheet& sheet, const pb::RectI& rect, const google::protobuf::RepeatedPtrField<pb::RectI>& rect_pieces,
    bool lightning) {
	auto [dst_surface, dst_rect] = alloc(rect.w(), rect.h());

	for (const auto& rect_piece : rect_pieces) {
		auto blit_src_rect = static_cast<SDL_Rect>(RectI{rect_piece});
		auto blit_dst_rect = SDL_Rect{
		    dst_rect.x + rect_piece.x() - rect.x(), dst_rect.y + rect_piece.y() - rect.y(), rect_piece.w(),
		    rect_piece.h()};
		SDL_BlitSurface(sheet.surface(), &blit_src_rect, dst_surface, &blit_dst_rect);
	}

	recreate_texture(lightning);

	return dst_rect;
}
m2::RectI m2::SpriteEffectsSheet::create_grayscale_effect(
    const SpriteSheet& sheet, const pb::RectI& rect, bool lightning) {
	auto [dst_surface, dst_rect] = alloc(rect.w(), rect.h());

	// Check pixel stride
	auto* src_surface = sheet.surface();
	if (src_surface->format->BytesPerPixel != 4 || dst_surface->format->BytesPerPixel != 4) {
		throw M2FATAL("Surface has unsupported pixel format");
	}

	SDL_LockSurface(src_surface);
	SDL_LockSurface(dst_surface);

	for (int y = rect.y(); y < rect.y() + rect.h(); ++y) {
		for (int x = rect.x(); x < rect.x() + rect.w(); ++x) {
			// Read src pixel
			auto* src_pixels = static_cast<uint32_t*>(src_surface->pixels);
			auto src_pixel = *(src_pixels + (x + y * src_surface->w));
			// Decompose to RPG
			uint8_t r, g, b, a;
			SDL_GetRGBA(src_pixel, src_surface->format, &r, &g, &b, &a);
			// Apply weights
			float rf = 0.299f * static_cast<float>(r) / 255.0f;
			float gf = 0.587f * static_cast<float>(g) / 255.0f;
			float bf = 0.114f * static_cast<float>(b) / 255.0f;
			// Convert back to int
			auto bw = (uint8_t)roundf((rf + gf + bf) * 255.0f);
			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dst_surface->pixels);
			auto* dst_pixel = dst_pixels + ((x - rect.x()) + (y - rect.y() + dst_rect.y) * dst_surface->w);
			*dst_pixel = SDL_MapRGBA(dst_surface->format, bw, bw, bw, a);
		}
	}

	SDL_UnlockSurface(dst_surface);
	SDL_UnlockSurface(src_surface);

	recreate_texture(lightning);

	return dst_rect;
}
m2::RectI m2::SpriteEffectsSheet::create_image_adjustment_effect(
    const SpriteSheet& sheet, const pb::RectI& rect, const pb::ImageAdjustment& image_adjustment, bool lightning) {
	auto [dst_surface, dst_rect] = alloc(rect.w(), rect.h());

	// Check pixel stride
	auto* src_surface = sheet.surface();
	if (src_surface->format->BytesPerPixel != 4 || dst_surface->format->BytesPerPixel != 4) {
		throw M2FATAL("Surface has unsupported pixel format");
	}

	SDL_LockSurface(src_surface);
	SDL_LockSurface(dst_surface);

	for (int y = rect.y(); y < rect.y() + rect.h(); ++y) {
		for (int x = rect.x(); x < rect.x() + rect.w(); ++x) {
			// Read src pixel
			auto* src_pixels = static_cast<uint32_t*>(src_surface->pixels);
			auto src_pixel = *(src_pixels + (x + y * src_surface->w));
			// Decompose to RPG
			uint8_t r, g, b, a;
			SDL_GetRGBA(src_pixel, src_surface->format, &r, &g, &b, &a);
			// Apply weights
			float rf = image_adjustment.brightness_multiplier() * static_cast<float>(r) / 255.0f;
			float gf = image_adjustment.brightness_multiplier() * static_cast<float>(g) / 255.0f;
			float bf = image_adjustment.brightness_multiplier() * static_cast<float>(b) / 255.0f;
			// Convert back to int
			auto rn = (uint8_t)roundf(rf * 255.0f);
			auto gn = (uint8_t)roundf(gf * 255.0f);
			auto bn = (uint8_t)roundf(bf * 255.0f);
			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dst_surface->pixels);
			auto* dst_pixel = dst_pixels + ((x - rect.x()) + (y - rect.y() + dst_rect.y) * dst_surface->w);
			*dst_pixel = SDL_MapRGBA(dst_surface->format, rn, gn, bn, a);
		}
	}

	SDL_UnlockSurface(dst_surface);
	SDL_UnlockSurface(src_surface);

	recreate_texture(lightning);

	return dst_rect;
}

m2::Sprite::Sprite(
    const SpriteSheet& sprite_sheet, SpriteEffectsSheet& sprite_effects_sheet, const pb::Sprite& sprite, bool lightning)
    : _sprite_sheet(&sprite_sheet),
      _effects_sheet(&sprite_effects_sheet),
      _sprite(sprite),
      _rect(sprite.regular().rect()),
      _original_rotation_radians(sprite.regular().original_rotation() * m2::PI),
      _ppm(sprite.regular().override_ppm() ? sprite.regular().override_ppm() : sprite_sheet.sprite_sheet().ppm()),
      _center_to_origin_vec_px(sprite.regular().center_to_origin_vec_px()),
      _center_offset_m(_center_to_origin_vec_px / (float)_ppm),
      _background_collider_type(
          sprite.regular().has_background_collider()
              ? (sprite.regular().background_collider().has_rect_dims_px() ? box2d::ColliderType::RECTANGLE
                                                                           : box2d::ColliderType::CIRCLE)
              : box2d::ColliderType::NONE),
      _background_collider_origin_to_origin_vec_m(
          VecF{sprite.regular().background_collider().origin_to_origin_vec_px()} / (float)_ppm),
      _background_collider_rect_dims_m(VecF{sprite.regular().background_collider().rect_dims_px()} / (float)_ppm),
      _background_collider_circ_radius_m(sprite.regular().background_collider().circ_radius_px() / (float)_ppm),
      _foreground_collider_type(
          sprite.regular().has_foreground_collider()
              ? (sprite.regular().foreground_collider().has_rect_dims_px() ? box2d::ColliderType::RECTANGLE
                                                                           : box2d::ColliderType::CIRCLE)
              : box2d::ColliderType::NONE),
      _foreground_collider_origin_to_origin_vec_m(
          VecF{sprite.regular().foreground_collider().origin_to_origin_vec_px()} /
          (float)_ppm),  // TODO rename to _foreground_collider_origin_offset_m
      _foreground_collider_rect_dims_m(VecF{sprite.regular().foreground_collider().rect_dims_px()} / (float)_ppm),
      _foreground_collider_circ_radius_m(sprite.regular().foreground_collider().circ_radius_px() / (float)_ppm),
      _is_background_tile(sprite.regular().is_background_tile()) {
	// Create foreground companion
	if (sprite.has_regular() && sprite.regular().foreground_companion_rects_size()) {
		_foreground_companion_sprite_effects_sheet_rect = sprite_effects_sheet.create_foreground_companion_effect(
		    sprite_sheet, sprite.regular().rect(), sprite.regular().foreground_companion_rects(), lightning);
		_foreground_companion_center_to_origin_vec_px =
		    VecF{sprite.regular().foreground_companion_center_to_origin_vec_px()};
		_foreground_companion_center_to_origin_vec_m =
		    VecF{sprite.regular().foreground_companion_center_to_origin_vec_px()} / (float)_ppm;
	}
	// Create effects
	if (sprite.has_regular() && sprite.regular().effects_size()) {
		_effects.resize(pb::enum_value_count<pb::SpriteEffectType>());
		std::vector<bool> is_created(pb::enum_value_count<pb::SpriteEffectType>());
		for (const auto& effect : sprite.regular().effects()) {
			auto index = pb::enum_index(effect.type());
			// Check if the effect is already created
			if (is_created[index]) {
				throw M2ERROR("Sprite has duplicate effect definition: " + std::to_string(effect.type()));
			}
			// Create effect
			switch (effect.type()) {
				case pb::SPRITE_EFFECT_MASK:
					_effects[index] = sprite_effects_sheet.create_mask_effect(
					    sprite_sheet, sprite.regular().rect(), effect.mask_color(), lightning);
					break;
				case pb::SPRITE_EFFECT_GRAYSCALE:
					_effects[index] =
					    sprite_effects_sheet.create_grayscale_effect(sprite_sheet, sprite.regular().rect(), lightning);
					break;
				case pb::SPRITE_EFFECT_IMAGE_ADJUSTMENT:
					_effects[index] = sprite_effects_sheet.create_image_adjustment_effect(
					    sprite_sheet, sprite.regular().rect(), effect.image_adjustment(), lightning);
					break;
				default:
					throw M2ERROR("Encountered a sprite with unknown sprite effect: " + std::to_string(sprite.type()));
			}
			is_created[index] = true;
		}
	}
}

SDL_Texture* m2::Sprite::texture(DrawVariant draw_variant) const {
	if (std::holds_alternative<IsForegroundCompanion>(draw_variant) &&
	    not std::get<IsForegroundCompanion>(draw_variant)) {
		return sprite_sheet().texture();
	} else {
		return effects_texture();
	}
}

m2::VecF m2::Sprite::texture_total_dimensions(DrawVariant draw_variant) const {
	if (std::holds_alternative<IsForegroundCompanion>(draw_variant) &&
	    not std::get<IsForegroundCompanion>(draw_variant)) {
		return {sprite_sheet().surface()->w, sprite_sheet().surface()->h};
	} else {
		return {effects_sheet()->texture_width(), effects_sheet()->texture_height()};
	}
}

const m2::RectI& m2::Sprite::rect(DrawVariant draw_variant) const {
	if (std::holds_alternative<IsForegroundCompanion>(draw_variant)) {
		if (std::get<IsForegroundCompanion>(draw_variant)) {
			return *_foreground_companion_sprite_effects_sheet_rect;
		} else {
			return rect();
		}
	} else {
		return effect_rect(std::get<pb::SpriteEffectType>(draw_variant));
	}
}

float m2::Sprite::sheet_to_screen_pixel_multiplier() const {
	return static_cast<float>(GAME.dimensions().ppm) / static_cast<float>(ppm());
}
m2::VecF m2::Sprite::center_to_origin_srcpx(DrawVariant draw_variant) const {
	if (std::holds_alternative<IsForegroundCompanion>(draw_variant) && std::get<IsForegroundCompanion>(draw_variant)) {
		return foreground_companion_center_to_origin_vec_px();
	} else {
		return original_rotation_radians() != 0.0f ? center_to_origin_vec_px().rotate(original_rotation_radians())
		                                           : center_to_origin_vec_px();
	}
}

std::vector<m2::SpriteSheet> m2::load_sprite_sheets(
    const std::filesystem::path& sprite_sheets_path, SDL_Renderer* renderer, bool lightning) {
	auto sheets = pb::json_file_to_message<pb::SpriteSheets>(sprite_sheets_path);
	if (!sheets) {
		throw M2ERROR(sheets.error());
	}
	std::vector<m2::SpriteSheet> sheets_vector;
	std::for_each(sheets->sheets().begin(), sheets->sheets().end(), [&](const auto& sheet) {
		sheets_vector.emplace_back(sheet, renderer, lightning);
	});
	return sheets_vector;
}

std::vector<m2::Sprite> m2::load_sprites(
    const std::vector<SpriteSheet>& sprite_sheets, SpriteEffectsSheet& sprite_effects_sheet, bool lightning) {
	std::vector<Sprite> sprites_vector(pb::enum_value_count<m2g::pb::SpriteType>());
	std::vector<bool> is_loaded(pb::enum_value_count<m2g::pb::SpriteType>());

	// Load sprites
	for (const auto& sprite_sheet : sprite_sheets) {
		for (const auto& sprite : sprite_sheet.sprite_sheet().sprites()) {
			auto index = pb::enum_index(sprite.type());
			// Check if the sprite is already loaded
			if (is_loaded[index]) {
				throw M2ERROR("Sprite has duplicate definition: " + std::to_string(sprite.type()));
			}
			// Load sprite
			sprites_vector[index] = Sprite{sprite_sheet, sprite_effects_sheet, sprite, lightning};
			is_loaded[index] = true;
		}
	}

	// Check if every sprite type is loaded
	for (int e = 0; e < pb::enum_value_count<m2g::pb::SpriteType>(); ++e) {
		if (!is_loaded[e]) {
			throw M2ERROR("Sprite is not defined: " + pb::enum_name<m2g::pb::SpriteType>(e));
		}
	}

	return sprites_vector;
}

std::vector<m2g::pb::SpriteType> m2::list_level_editor_background_sprites(
    const std::vector<SpriteSheet>& sprite_sheets) {
	std::vector<m2g::pb::SpriteType> sprites_vector;

	for (const auto& sprite_sheet : sprite_sheets) {
		for (const auto& sprite : sprite_sheet.sprite_sheet().sprites()) {
			if (sprite.regular().is_background_tile()) {
				sprites_vector.push_back(sprite.type());
			}
		}
	}

	return sprites_vector;
}

std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> m2::list_level_editor_object_sprites(
    const std::filesystem::path& objects_path) {
	auto objects = pb::json_file_to_message<pb::Objects>(objects_path);
	if (!objects) {
		throw M2ERROR(objects.error());
	}

	std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> object_sprite_map;
	std::vector<bool> has_encountered(pb::enum_value_count<m2g::pb::ObjectType>());

	// Visit every object
	for (const auto& object : objects->objects()) {
		auto index = pb::enum_index(object.type());
		// Check if object type already exists
		if (has_encountered[index]) {
			throw M2ERROR("Object has duplicate definition: " + std::to_string(object.type()));
		}
		has_encountered[index] = true;

		if (object.main_sprite()) {
			object_sprite_map[object.type()] = object.main_sprite();
		}
	}

	// Check if every object type is encountered
	for (int e = 0; e < pb::enum_value_count<m2g::pb::ObjectType>(); ++e) {
		if (!has_encountered[e]) {
			throw M2ERROR("Object is not defined: " + pb::enum_name<m2g::pb::ObjectType>(e));
		}
	}

	return object_sprite_map;
}
