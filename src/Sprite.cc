#include <m2/Sprite.h>
#include <m2/Proto.h>
#include <m2/Exception.h>
#include <sstream>

std::unordered_map<std::string, m2::pb::SpriteSheet> m2::sprite::load_sprite_sheets(const std::string& sprite_sheets_path) {
	auto sheets = proto::json_file_to_message<pb::SpriteSheets>(sprite_sheets_path);
	if (!sheets) {
		throw M2ERROR(sheets.error());
	}

	std::unordered_map<std::string, m2::pb::SpriteSheet> sheets_map;
	for (const auto& sheet : sheets->sheets()) {
		sheets_map[sheet.key()] = sheet;
	}
	return sheets_map;
}

std::unordered_map<std::string, m2::pb::Sprite> m2::sprite::load_sprites(const std::string& sprite_sheets_path) {
	auto sheets = proto::json_file_to_message<pb::SpriteSheets>(sprite_sheets_path);
	if (!sheets) {
		throw M2ERROR(sheets.error());
	}

	std::unordered_map<std::string, m2::pb::Sprite> sprites_map;
	for (const auto& sheet : sheets->sheets()) {
		for (const auto& sprite : sheet.sprites()) {
			std::stringstream ss;
			ss << sheet.key() << '.' << sprite.key();
			sprites_map[ss.str()] = sprite;
		}
	}
	return sprites_map;
}
