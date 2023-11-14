#include <m2/Sprite.h>
#include <m2/protobuf/Detail.h>
#include <Object.pb.h>
#include <m2/M2.h>
#include <m2/Proxy.h>
#include <m2/Exception.h>
#include <SDL2/SDL_image.h>
#include <m2/Game.h>

m2::SpriteSheet::SpriteSheet(const pb::SpriteSheet& sprite_sheet, SDL_Renderer* renderer) : _sprite_sheet(sprite_sheet) {
	_surface.reset(IMG_Load(sprite_sheet.resource().c_str()));
	if (not _surface) {
		throw M2ERROR("SDL Error while loading " + sprite_sheet.resource() + ": " + IMG_GetError());
	}
	_texture.reset(SDL_CreateTextureFromSurface(renderer, _surface.get()));
	if (not _texture) {
		throw M2ERROR("SDL Error while creating texture from surface" + sprite_sheet.resource() + ": " + IMG_GetError());
	}
	SDL_SetTextureBlendMode(_texture.get(), SDL_BLENDMODE_BLEND);
	if (m2g::lightning) {
		// Darken the texture
		SDL_SetTextureColorMod(_texture.get(), 127, 127, 127);
	}
}
const m2::pb::SpriteSheet& m2::SpriteSheet::sprite_sheet() const {
	return _sprite_sheet;
}
SDL_Surface* m2::SpriteSheet::surface() const {
	return _surface.get(); // TODO potentially dangerous, use shared_ptr instead
}
SDL_Texture* m2::SpriteSheet::texture() const {
	return _texture.get(); // TODO potentially dangerous, use shared_ptr instead
}

m2::SpriteEffectsSheet::SpriteEffectsSheet(SDL_Renderer* renderer) : DynamicSheet(renderer) {}
m2::RectI m2::SpriteEffectsSheet::create_mask_effect(const SpriteSheet &sheet, const pb::RectI &rect, const pb::Color& mask_color) {
	auto [dst_surface, dst_rect] = alloc(rect.w(), rect.h());

	// Check pixel stride
	auto* src_surface = sheet.surface();
	if (src_surface->format->BytesPerPixel != 4 || dst_surface->format->BytesPerPixel != 4) {
		throw M2FATAL("Surface has unsupported pixel format");
	}

	// Prepare mask color
	uint32_t dst_color = ((mask_color.r() >> dst_surface->format->Rloss) << dst_surface->format->Rshift) & dst_surface->format->Rmask;
	dst_color |= ((mask_color.g() >> dst_surface->format->Gloss) << dst_surface->format->Gshift) & dst_surface->format->Gmask;
	dst_color |= ((mask_color.b() >> dst_surface->format->Bloss) << dst_surface->format->Bshift) & dst_surface->format->Bmask;
	dst_color |= ((mask_color.a() >> dst_surface->format->Aloss) << dst_surface->format->Ashift) & dst_surface->format->Amask;

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

	recreate_texture();

	return dst_rect;
}
m2::RectI m2::SpriteEffectsSheet::create_foreground_companion_effect(const SpriteSheet& sheet, const pb::RectI& rect, const google::protobuf::RepeatedPtrField<pb::RectI>& rect_pieces) {
	auto [dst_surface, dst_rect] = alloc(rect.w(), rect.h());

	for (const auto& rect_piece : rect_pieces) {
		auto blit_src_rect = sdl::to_rect(rect_piece);
		auto blit_dst_rect = SDL_Rect{
			dst_rect.x + rect_piece.x() - rect.x(),
			dst_rect.y + rect_piece.y() - rect.y(),
			rect_piece.w(),
			rect_piece.h()
		};
		SDL_BlitSurface(sheet.surface(), &blit_src_rect, dst_surface, &blit_dst_rect);
	}

	recreate_texture();

	return dst_rect;
}
m2::RectI m2::SpriteEffectsSheet::create_grayscale_effect(const SpriteSheet& sheet, const pb::RectI &rect) {
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
			uint8_t r,g,b,a;
			SDL_GetRGBA(src_pixel, src_surface->format, &r, &g, &b, &a);
			// Apply weights
			float rf = 0.299f * static_cast<float>(r) / 255.0f;
			float gf = 0.587f * static_cast<float>(g) / 255.0f;
			float bf = 0.114f * static_cast<float>(b) / 255.0f;
			// Convert back to int
			auto bw = (uint8_t) roundf((rf + gf + bf) * 255.0f);
			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dst_surface->pixels);
			auto* dst_pixel = dst_pixels + ((x - rect.x()) + (y - rect.y() + dst_rect.y) * dst_surface->w);
			*dst_pixel = SDL_MapRGBA(dst_surface->format, bw, bw, bw, a);
		}
	}

	SDL_UnlockSurface(dst_surface);
	SDL_UnlockSurface(src_surface);

	recreate_texture();

	return dst_rect;
}
m2::RectI m2::SpriteEffectsSheet::create_image_adjustment_effect(const SpriteSheet& sheet, const pb::RectI &rect, const pb::ImageAdjustment& image_adjustment) {
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
			uint8_t r,g,b,a;
			SDL_GetRGBA(src_pixel, src_surface->format, &r, &g, &b, &a);
			// Apply weights
			float rf = image_adjustment.brightness_multiplier() * static_cast<float>(r) / 255.0f;
			float gf = image_adjustment.brightness_multiplier() * static_cast<float>(g) / 255.0f;
			float bf = image_adjustment.brightness_multiplier() * static_cast<float>(b) / 255.0f;
			// Convert back to int
			auto rn = (uint8_t) roundf(rf * 255.0f);
			auto gn = (uint8_t) roundf(gf * 255.0f);
			auto bn = (uint8_t) roundf(bf * 255.0f);
			// Color dst pixel
			auto* dst_pixels = static_cast<uint32_t*>(dst_surface->pixels);
			auto* dst_pixel = dst_pixels + ((x - rect.x()) + (y - rect.y() + dst_rect.y) * dst_surface->w);
			*dst_pixel = SDL_MapRGBA(dst_surface->format, rn, gn, bn, a);
		}
	}

	SDL_UnlockSurface(dst_surface);
	SDL_UnlockSurface(src_surface);

	recreate_texture();

	return dst_rect;
}

m2::Sprite::Sprite(const SpriteSheet& sprite_sheet, SpriteEffectsSheet& sprite_effects_sheet, const pb::Sprite& sprite) :
	_sprite_sheet(&sprite_sheet),
	_sprite(sprite),
	_effects_sheet(&sprite_effects_sheet),
	_rect(sprite.regular().rect()),
	_original_rotation_radians(sprite.regular().original_rotation() * m2::PI),
	_ppm(sprite.regular().override_ppm() ? sprite.regular().override_ppm() : sprite_sheet.sprite_sheet().ppm()),
	_center_offset_px(sprite.regular().center_offset_px()),
	_center_offset_m(_center_offset_px / (float) _ppm),
	_background_collider_type(sprite.regular().has_background_collider()
			? (sprite.regular().background_collider().has_rect_dims_px() ? box2d::ColliderType::RECTANGLE : box2d::ColliderType::CIRCLE)
			: box2d::ColliderType::NONE),
	_background_collider_center_offset_m(VecF{sprite.regular().background_collider().origin_offset_px()} / (float)_ppm), // TODO rename to _background_collider_origin_offset_m
	_background_collider_rect_dims_m(VecF{sprite.regular().background_collider().rect_dims_px()} / (float)_ppm),
	_background_collider_circ_radius_m(sprite.regular().background_collider().circ_radius_px() / (float)_ppm),
	_foreground_collider_type(sprite.regular().has_foreground_collider()
				? (sprite.regular().foreground_collider().has_rect_dims_px() ? box2d::ColliderType::RECTANGLE : box2d::ColliderType::CIRCLE)
				: box2d::ColliderType::NONE),
	_foreground_collider_center_offset_m(VecF{sprite.regular().foreground_collider().origin_offset_px()} / (float)_ppm), // TODO rename to _foreground_collider_origin_offset_m
	_foreground_collider_rect_dims_m(VecF{sprite.regular().foreground_collider().rect_dims_px()} / (float)_ppm),
	_foreground_collider_circ_radius_m(sprite.regular().foreground_collider().circ_radius_px() / (float)_ppm),
	_is_background_tile(sprite.regular().is_background_tile()) {
	// Create effects
	if (sprite.has_regular() && sprite.regular().effects_size()) {
		_effects.resize(protobuf::enum_value_count<pb::SpriteEffectType>());
		std::vector<bool> is_created(protobuf::enum_value_count<pb::SpriteEffectType>());
		for (const auto& effect : sprite.regular().effects()) {
			auto index = protobuf::enum_index(effect.type());
			// Check if the effect is already created
			if (is_created[index]) {
				throw M2ERROR("Sprite has duplicate effect definition: " + std::to_string(effect.type()));
			}
			// Create effect
			switch (effect.type()) {
				case pb::SPRITE_EFFECT_FOREGROUND_COMPANION:
					_effects[index] = sprite_effects_sheet.create_foreground_companion_effect(sprite_sheet, sprite.regular().rect(), effect.foreground_companion().rects());
					_foreground_companion_center_offset_px = VecF{effect.foreground_companion().center_offset_px()};
					_foreground_companion_center_offset_m = VecF{effect.foreground_companion().center_offset_px()} / (float)_ppm;
					break;
				case pb::SPRITE_EFFECT_MASK:
					_effects[index] = sprite_effects_sheet.create_mask_effect(sprite_sheet, sprite.regular().rect(), effect.mask_color());
					break;
				case pb::SPRITE_EFFECT_GRAYSCALE:
					_effects[index] = sprite_effects_sheet.create_grayscale_effect(sprite_sheet, sprite.regular().rect());
					break;
				case pb::SPRITE_EFFECT_IMAGE_ADJUSTMENT:
					_effects[index] = sprite_effects_sheet.create_image_adjustment_effect(sprite_sheet, sprite.regular().rect(), effect.image_adjustment());
					break;
				default:
					throw M2ERROR("Encountered a sprite with unknown sprite effect: " + std::to_string(sprite.type()));
			}
			is_created[index] = true;
		}
	}
}

const m2::SpriteSheet& m2::Sprite::sprite_sheet() const {
	return *_sprite_sheet;
}
SDL_Texture* m2::Sprite::effects_texture() const {
	if (_effects_sheet) {
		return _effects_sheet->texture();
	}
	return nullptr;
}
m2::RectI m2::Sprite::effect_rect(pb::SpriteEffectType effect_type) const {
	return _effects[protobuf::enum_index(effect_type)];
}
bool m2::Sprite::has_foreground_companion() const {
	return _foreground_companion_center_offset_m.has_value();
}
m2::VecF m2::Sprite::foreground_companion_center_offset_px() const {
	return _foreground_companion_center_offset_px.value();
}
m2::VecF m2::Sprite::foreground_companion_center_offset_m() const {
	return _foreground_companion_center_offset_m.value();
}
float m2::Sprite::original_rotation_radians() const {
	return _original_rotation_radians;
}
int m2::Sprite::ppm() const {
	return _ppm;
}
m2::VecF m2::Sprite::background_collider_center_offset_m() const {
	return _background_collider_center_offset_m;
}
m2::VecF m2::Sprite::background_collider_rect_dims_m() const {
	return _background_collider_rect_dims_m;
}
float m2::Sprite::background_collider_circ_radius_m() const {
	return _background_collider_circ_radius_m;
}
m2::VecF m2::Sprite::foreground_collider_center_offset_m() const {
	return _foreground_collider_center_offset_m;
}
m2::VecF m2::Sprite::foreground_collider_rect_dims_m() const {
	return _foreground_collider_rect_dims_m;
}
float m2::Sprite::foreground_collider_circ_radius_m() const {
	return _foreground_collider_circ_radius_m;
}

float m2::Sprite::sheet_to_screen_pixel_multiplier() const {
	return static_cast<float>(GAME.dimensions().ppm) / static_cast<float>(ppm());
}
m2::VecF m2::Sprite::center_to_origin_srcpx(pb::SpriteEffectType effect_type) const {
	if (effect_type == pb::SPRITE_EFFECT_FOREGROUND_COMPANION && has_foreground_companion()) {
		return foreground_companion_center_offset_px();
	} else {
		return original_rotation_radians() != 0.0f
				? center_offset_px().rotate(original_rotation_radians())
				: center_offset_px();
	}
}

std::vector<m2::SpriteSheet> m2::load_sprite_sheets(const std::filesystem::path& sprite_sheets_path, SDL_Renderer *renderer) {
	auto sheets = protobuf::json_file_to_message<pb::SpriteSheets>(sprite_sheets_path);
	if (!sheets) {
		throw M2ERROR(sheets.error());
	}
	std::vector<m2::SpriteSheet> sheets_vector;
	std::for_each(sheets->sheets().begin(), sheets->sheets().end(), [&](const auto& sheet) {
		sheets_vector.emplace_back(sheet, renderer);
	});
	return sheets_vector;
}

std::vector<m2::Sprite> m2::load_sprites(const std::vector<SpriteSheet>& sprite_sheets, SpriteEffectsSheet& sprite_effects_sheet) {
	std::vector<Sprite> sprites_vector(protobuf::enum_value_count<m2g::pb::SpriteType>());
	std::vector<bool> is_loaded(protobuf::enum_value_count<m2g::pb::SpriteType>());

	// Load sprites
	for (const auto& sprite_sheet : sprite_sheets) {
		for (const auto& sprite : sprite_sheet.sprite_sheet().sprites()) {
			auto index = protobuf::enum_index(sprite.type());
			// Check if the sprite is already loaded
			if (is_loaded[index]) {
				throw M2ERROR("Sprite has duplicate definition: " + std::to_string(sprite.type()));
			}
			// Load sprite
			sprites_vector[index] = Sprite{sprite_sheet, sprite_effects_sheet, sprite};
			is_loaded[index] = true;
		}
	}

	// Check if every sprite type is loaded
	for (int e = 0; e < protobuf::enum_value_count<m2g::pb::SpriteType>(); ++e) {
		if (!is_loaded[e]) {
			throw M2ERROR("Sprite is not defined: " + protobuf::enum_name<m2g::pb::SpriteType>(e));
		}
	}

	return sprites_vector;
}

std::vector<m2g::pb::SpriteType> m2::list_level_editor_background_sprites(const std::vector<SpriteSheet>& sprite_sheets) {
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

std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> m2::list_level_editor_object_sprites(const std::filesystem::path& objects_path) {
	auto objects = protobuf::json_file_to_message<pb::Objects>(objects_path);
	if (!objects) {
		throw M2ERROR(objects.error());
	}

	std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> object_sprite_map;
	std::vector<bool> has_encountered(protobuf::enum_value_count<m2g::pb::ObjectType>());

	// Visit every object
	for (const auto& object : objects->objects()) {
		auto index = protobuf::enum_index(object.type());
		// Check if object type already exists
		if (has_encountered[index]) {
			throw M2ERROR("Object has duplicate definition: " + std::to_string(object.type()));
		}
		has_encountered[index] = true;

		if (object.level_editor_sprite_type()) {
			object_sprite_map[object.type()] = object.level_editor_sprite_type();
		}
	}

	// Check if every object type is encountered
	for (int e = 0; e < protobuf::enum_value_count<m2g::pb::ObjectType>(); ++e) {
		if (!has_encountered[e]) {
			throw M2ERROR("Object is not defined: " + protobuf::enum_name<m2g::pb::ObjectType>(e));
		}
	}

	return object_sprite_map;
}
