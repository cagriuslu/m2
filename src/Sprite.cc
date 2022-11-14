#include <m2/Sprite.h>
#include <m2/protobuf/Utils.h>
#include <Object.pb.h>
#include <m2/Exception.h>
#include <SDL2/SDL_image.h>

m2::SpriteSheet::SpriteSheet(const pb::SpriteSheet& sprite_sheet, SDL_Renderer* renderer) : _sprite_sheet(sprite_sheet) {
	_surface.reset(IMG_Load(sprite_sheet.resource().c_str()));
	if (not _surface) {
		throw M2ERROR("SDL Error while loading " + sprite_sheet.resource() + ": " + IMG_GetError());
	}
	_texture.reset(SDL_CreateTextureFromSurface(renderer, _surface.get()));
	if (not _texture) {
		throw M2ERROR("SDL Error while creating texture from surface" + sprite_sheet.resource() + ": " + IMG_GetError());
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
SDL_Rect m2::SpriteEffectsSheet::create_mask_effect(const SpriteSheet &sheet, const pb::Rect2i &rect, const pb::Color& mask_color) {
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
SDL_Rect m2::SpriteEffectsSheet::create_foreground_companion_effect(const SpriteSheet& sheet, const pb::Rect2i &rect, const google::protobuf::RepeatedPtrField<pb::Rect2i>& rect_pieces) {
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

m2::Sprite::Sprite(const SpriteSheet& sprite_sheet, SpriteEffectsSheet& sprite_effects_sheet, const pb::Sprite& sprite) :
	_sprite_sheet(&sprite_sheet), _sprite(sprite), _effects_sheet(&sprite_effects_sheet),
	_ppm(sprite.override_ppm() ? (int)sprite.override_ppm() : (int)sprite_sheet.sprite_sheet().ppm()),
	_center_offset_m(Vec2f{sprite.center_offset_px()} / _ppm),
	_background_collider_center_offset_m(Vec2f{sprite.background_collider().center_offset_px()} / _ppm),
	_background_collider_rect_dims_m(Vec2f{sprite.background_collider().rect_dims_px()} / _ppm),
	_background_collider_circ_radius_m(sprite.background_collider().circ_radius_px() / (float)_ppm),
	_foreground_collider_center_offset_m(Vec2f{sprite.foreground_collider().center_offset_px()} / _ppm),
	_foreground_collider_rect_dims_m(Vec2f{sprite.foreground_collider().rect_dims_px()} / _ppm),
	_foreground_collider_circ_radius_m(sprite.foreground_collider().circ_radius_px() / (float)_ppm) {
	// Create effects
	if (sprite.effects_size()) {
		_effects.resize(pb::SpriteEffectType_ARRAYSIZE);
		std::vector<bool> is_created(pb::SpriteEffectType_ARRAYSIZE);
		for (const auto& effect : sprite.effects()) {
			// Check if the effect is already created
			if (is_created[effect.type()]) {
				throw M2ERROR("Sprite has duplicate effect definition: " + std::to_string(effect.type()));
			}
			// Create effect
			switch (effect.type()) {
				case pb::SPRITE_EFFECT_FOREGROUND_COMPANION:
					_effects[effect.type()] = sprite_effects_sheet.create_foreground_companion_effect(sprite_sheet, sprite.rect(), effect.foreground_companion().rects());
					_foreground_companion_center_offset_px = Vec2f{effect.foreground_companion().center_offset_px()};
					_foreground_companion_center_offset_m = Vec2f{effect.foreground_companion().center_offset_px()} / (float)_ppm;
					break;
				case pb::SPRITE_EFFECT_MASK:
					_effects[effect.type()] = sprite_effects_sheet.create_mask_effect(sprite_sheet, sprite.rect(), effect.mask_color());
					break;
				default:
					break;
			}
			is_created[effect.type()] = true;
		}
	}
}
const m2::SpriteSheet& m2::Sprite::sprite_sheet() const {
	return *_sprite_sheet;
}
const m2::pb::Sprite& m2::Sprite::sprite() const {
	return _sprite;
}
SDL_Texture* m2::Sprite::effects_texture() const {
	if (_effects_sheet) {
		return _effects_sheet->texture();
	}
	return nullptr;
}
SDL_Rect m2::Sprite::effect_rect(pb::SpriteEffectType effect_type) const {
	return _effects[effect_type];
}
bool m2::Sprite::has_foreground_companion() const {
	return _foreground_companion_center_offset_m.has_value();
}
m2::Vec2f m2::Sprite::foreground_companion_center_offset_px() const {
	return _foreground_companion_center_offset_px.value();
}
m2::Vec2f m2::Sprite::foreground_companion_center_offset_m() const {
	return _foreground_companion_center_offset_m.value();
}
int m2::Sprite::ppm() const {
	return _ppm;
}
m2::Vec2f m2::Sprite::center_offset_m() const {
	return _center_offset_m;
}
m2::Vec2f m2::Sprite::background_collider_center_offset_m() const {
	return _background_collider_center_offset_m;
}
m2::Vec2f m2::Sprite::background_collider_rect_dims_m() const {
	return _background_collider_rect_dims_m;
}
float m2::Sprite::background_collider_circ_radius_m() const {
	return _background_collider_circ_radius_m;
}
m2::Vec2f m2::Sprite::foreground_collider_center_offset_m() const {
	return _foreground_collider_center_offset_m;
}
m2::Vec2f m2::Sprite::foreground_collider_rect_dims_m() const {
	return _foreground_collider_rect_dims_m;
}
float m2::Sprite::foreground_collider_circ_radius_m() const {
	return _foreground_collider_circ_radius_m;
}

std::vector<m2::SpriteSheet> m2::load_sprite_sheets(const std::string &sprite_sheets_path, SDL_Renderer *renderer) {
	auto sheets = proto::json_file_to_message<pb::SpriteSheets>(sprite_sheets_path);
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
	std::vector<Sprite> sprites_vector(m2g::pb::SpriteType_ARRAYSIZE);
	std::vector<bool> is_loaded(m2g::pb::SpriteType_ARRAYSIZE);

	// Load sprites
	for (const auto& sprite_sheet : sprite_sheets) {
		for (const auto& sprite : sprite_sheet.sprite_sheet().sprites()) {
			// Check if the sprite is already loaded
			if (is_loaded[sprite.type()]) {
				throw M2ERROR("Sprite has duplicate definition: " + std::to_string(sprite.type()));
			}
			// Load sprite
			sprites_vector[sprite.type()] = Sprite{sprite_sheet, sprite_effects_sheet, sprite};
			is_loaded[sprite.type()] = true;
		}
	}

	// Check if every sprite type is loaded
	const auto* sprite_type_desc = m2g::pb::SpriteType_descriptor();
	for (int e = 0; e < sprite_type_desc->value_count(); ++e) {
		int value = sprite_type_desc->value(e)->number();
		if (!is_loaded[value]) {
			throw M2ERROR("Sprite is not defined: " + std::to_string(value));
		}
	}

	return sprites_vector;
}

std::vector<m2g::pb::SpriteType> m2::list_editor_background_sprites(const std::vector<SpriteSheet>& sprite_sheets) {
	std::vector<m2g::pb::SpriteType> sprites_vector;

	for (const auto& sprite_sheet : sprite_sheets) {
		for (const auto& sprite : sprite_sheet.sprite_sheet().sprites()) {
			if (sprite.is_background_tile()) {
				sprites_vector.push_back(sprite.type());
			}
		}
	}

	return sprites_vector;
}

std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> m2::list_editor_object_sprites(const std::string& objects_path) {
	auto objects = proto::json_file_to_message<pb::Objects>(objects_path);
	if (!objects) {
		throw M2ERROR(objects.error());
	}

	std::map<m2g::pb::ObjectType, m2g::pb::SpriteType> object_sprite_map;
	std::vector<bool> has_encountered(m2g::pb::ObjectType_ARRAYSIZE);

	// Visit every object
	for (const auto& object : objects->objects()) {
		// Check if object type already exists
		if (has_encountered[object.type()]) {
			throw M2ERROR("Object has duplicate definition: " + std::to_string(object.type()));
		}
		has_encountered[object.type()] = true;

		if (object.editor_sprite_type()) {
			object_sprite_map[object.type()] = object.editor_sprite_type();
		}
	}

	// Check if every object type is encountered
	const auto* object_type_desc = m2g::pb::ObjectType_descriptor();
	for (int e = 0; e < object_type_desc->value_count(); ++e) {
		int value = object_type_desc->value(e)->number();
		if (!has_encountered[value]) {
			throw M2ERROR("Object is not defined: " +  std::to_string(value));
		}
	}

	return object_sprite_map;
}
