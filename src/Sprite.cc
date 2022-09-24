#include <m2/Sprite.h>
#include <m2/Proto.h>
#include <Object.pb.h>
#include <m2/Exception.h>
#include <SDL_image.h>

m2::SpriteSheet::SpriteSheet(const pb::SpriteSheet& sprite_sheet, SDL_Renderer* renderer) : _sprite_sheet(sprite_sheet) {
	SDL_Surface* surface = IMG_Load(sprite_sheet.resource().c_str());
	if (not surface) {
		throw M2ERROR("SDL Error while loading " + sprite_sheet.resource() + ": " + IMG_GetError());
	}
	_texture.reset(SDL_CreateTextureFromSurface(renderer, surface));
	if (not _texture) {
		throw M2ERROR("SDL Error while creating texture from surface" + sprite_sheet.resource() + ": " + IMG_GetError());
	}
	SDL_FreeSurface(surface);
}
const m2::pb::SpriteSheet& m2::SpriteSheet::sprite_sheet() const {
	return _sprite_sheet;
}
SDL_Texture* m2::SpriteSheet::texture() const {
	return _texture.get(); // TODO potentially dangerous, use shared_ptr instead
}

m2::Sprite::Sprite(const SpriteSheet& sprite_sheet, const pb::Sprite& sprite) :
	_sprite_sheet(&sprite_sheet), _sprite(sprite),
	_ppm(sprite.override_ppm() ? sprite.override_ppm() : sprite_sheet.sprite_sheet().ppm()),
	_center_offset_m(Vec2f{sprite.center_offset_px()} / _ppm),
	_collider_center_offset_m(Vec2f{sprite.collider().center_offset_px()} / _ppm),
	_collider_rect_dims_m(Vec2f{sprite.collider().rect_dims_px()} / _ppm),
	_collider_circ_radius_m(sprite.collider().circ_radius_px() / (float)_ppm) {}
const m2::SpriteSheet& m2::Sprite::sprite_sheet() const {
	return *_sprite_sheet;
}
const m2::pb::Sprite& m2::Sprite::sprite() const {
	return _sprite;
}
unsigned m2::Sprite::ppm() const {
	return _ppm;
}
m2::Vec2f m2::Sprite::center_offset_m() const {
	return _center_offset_m;
}
m2::Vec2f m2::Sprite::collider_center_offset_m() const {
	return _collider_center_offset_m;
}
m2::Vec2f m2::Sprite::collider_rect_dims_m() const {
	return _collider_rect_dims_m;
}
float m2::Sprite::collider_circ_radius_m() const {
	return _collider_circ_radius_m;
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

std::vector<m2::Sprite> m2::load_sprites(const std::vector<SpriteSheet>& sprite_sheets) {
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
			sprites_vector[sprite.type()] = Sprite{sprite_sheet, sprite};
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
