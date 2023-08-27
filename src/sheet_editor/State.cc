#include <m2/sheet_editor/State.h>
#include <m2/protobuf/Detail.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::sedit;

expected<m2::sedit::State> m2::sedit::State::create(const std::filesystem::path& path) {
	// If path exists,
	if (std::filesystem::exists(path)) {
		// Check if the file is a valid pb::SpriteSheets
		if (auto msg = protobuf::json_file_to_message<pb::SpriteSheets>(path); !msg) {
			return make_unexpected(msg.error());
		}
	}
	return State{path};
}

const m2::pb::SpriteSheets& m2::sedit::State::sprite_sheets() const {
	// If path exists,
	if (std::filesystem::exists(_path)) {
		// Check if the file is a valid pb::SpriteSheets
		if (auto msg = protobuf::json_file_to_message<pb::SpriteSheets>(_path); msg) {
			_sprite_sheets = *msg;
		} else {
			throw M2ERROR("File is not a valid m2::pb::SpriteSheets: " + _path.string());
		}
	} else {
		_sprite_sheets = {};
	}
	return _sprite_sheets;
}

void m2::sedit::State::select_sprite_type(m2g::pb::SpriteType sprite_type) {
	_selected_sprite_type = sprite_type;
}

void m2::sedit::State::prepare_sprite_selection() {
	const auto& sprite_sheets = this->sprite_sheets();

	// Reload dynamic image loader with the resource
	// To find sprite in the sheet, iterate over sheets
	for (const auto& sprite_sheet : sprite_sheets.sheets()) {
		// Iterate over sprites
		for (const auto& sprite : sprite_sheet.sprites()) {
			if (sprite.type() == _selected_sprite_type) {
				auto image_loader = DynamicImageLoader::create(sprite_sheet.resource());
				if (!image_loader) {
					throw M2ERROR("Failed to load the image: " + sprite_sheet.resource());
				}
				LEVEL.dynamic_image_loader.emplace(std::move(*image_loader));
				goto image_loader_loaded;
			}
		}
	}
	image_loader_loaded:
	return; // TODO remove
	// TODO
}
