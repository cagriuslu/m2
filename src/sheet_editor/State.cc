#include <m2/sheet_editor/State.h>
#include <m2/protobuf/Detail.h>

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
	// TODO
}
