#include <m2/Sprite.h>
#include <m2g/Sprite.h>
#include <m2/Proto.h>
#include <m2/Exception.h>
#include <m2/SDLUtils.hh>
#include <m2/M2.h>
#include <SDL_image.h>
#include <sstream>

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

m2::Sprite::Sprite(const SpriteSheet& sprite_sheet, const pb::Sprite& sprite, const std::string& key) :
	_sprite_sheet(sprite_sheet), _sprite(sprite), _key(key),
	_ppm(sprite.override_ppm() ? sprite.override_ppm() : sprite_sheet.sprite_sheet().ppm()),
	_center_offset_m(Vec2f{sprite.center_offset_px()} / _ppm),
	_collider_center_offset_m(Vec2f{sprite.collider().center_offset_px()} / _ppm),
	_collider_rect_dims_m(Vec2f{sprite.collider().rect_dims_px()} / _ppm),
	_collider_circ_radius_m(sprite.collider().circ_radius_px() / _ppm) {}
const m2::SpriteSheet& m2::Sprite::sprite_sheet() const {
	return _sprite_sheet;
}
const m2::pb::Sprite& m2::Sprite::sprite() const {
	return _sprite;
}
const std::string& m2::Sprite::key() const {
	return _key;
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

m2::SheetsAndSprites m2::load_sheets_and_sprites(const std::string& sprite_sheets_path, SDL_Renderer* renderer) {
	auto sheets = proto::json_file_to_message<pb::SpriteSheets>(sprite_sheets_path);
	if (!sheets) {
		throw M2ERROR(sheets.error());
	}
	SheetsAndSprites sheets_and_sprites;
	for (const auto& sheet : sheets->sheets()) {
		auto [sheets_map_it, sheet_inserted] = sheets_and_sprites.first.emplace(sheet.key(), SpriteSheet{sheet, renderer});
		if (not sheet_inserted) {
			throw M2ERROR("Sheets have duplicate keys");
		}
		for (const auto& sprite : sheet.sprites()) {
			std::stringstream ss;
			ss << sheet.key() << '.' << sprite.key();
			auto [sprites_map_it, sprite_inserted] = sheets_and_sprites.second.emplace(ss.str(), Sprite{sheets_map_it->second, sprite, ss.str()});
			if (not sprite_inserted) {
				throw M2ERROR("Sprites have duplicate keys");
			}
		}
	}
	return sheets_and_sprites;
}

m2::SpriteLut m2::generate_sprite_lut(const Sprites& sprites_map) {
	SpriteLut lut;
	// Iterate over m2g::sprite_lut
	unsigned counter = 0;
	for (const auto& [m2g_sprite, sprite_key] : m2g::sprite_lut) {
		if (to_unsigned(m2g_sprite) != counter++) {
			throw M2FATAL("m2g::sprite_lut contains out of order values");
		}
		auto sprites_map_it = sprites_map.find(sprite_key);
		if (sprites_map_it == sprites_map.end()) {
			throw M2FATAL("m2g::sprite_lut contains unknown sprite key");
		}
		lut.push_back(&sprites_map_it->second);
	}
	if (to_unsigned(m2g::Sprite::_end_) != counter) {
		throw M2FATAL("m2g::sprite_lut is incomplete");
	}
	return lut;
}
