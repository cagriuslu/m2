#include <m2/sheet_editor/State.h>
#include <m2/protobuf/Detail.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::sedit;

State::ForegroundCompanionMode::ForegroundCompanionMode() {
	const auto& sprite = LEVEL.sheet_editor_state->selected_sprite();
	// Iterate over effects
	for (const auto& effect : sprite.effects()) {
		if (effect.type() == pb::SpriteEffectType::SPRITE_EFFECT_FOREGROUND_COMPANION) {
			// Iterate over rects
			for (const auto& rect : effect.foreground_companion().rects()) {
				current_rects.emplace_back(RectI{rect});
			}
			// Set center
			current_center = VecF{effect.foreground_companion().center_offset_px()};
		}
	}
}
void State::ForegroundCompanionMode::add_rect() {
	// TODO
}
void State::ForegroundCompanionMode::set_center() {
	// TODO
}
void State::ForegroundCompanionMode::reset() {
	// TODO
}

State::RectMode::RectMode() {
	const auto& sprite = LEVEL.sheet_editor_state->selected_sprite();
	// Set rect
	current_rect = RectI{sprite.rect()};
	// Set center
	current_center = VecF{sprite.center_offset_px()};
}
void State::RectMode::set_rect() {
	// TODO
}
void State::RectMode::set_center() {
	// TODO
}
void State::RectMode::reset() {
	// TODO
}

State::BackgroundColliderMode::BackgroundColliderMode() {
	const auto& sprite = LEVEL.sheet_editor_state->selected_sprite();
	if (sprite.has_background_collider()) {
		auto collider_origin = VecF{sprite.center_offset_px()} + VecF{sprite.background_collider().center_offset_px()};
		if (sprite.background_collider().has_rect_dims_px()) {
			current_rect = RectF{collider_origin, VecF{sprite.background_collider().rect_dims_px()}};
		} else if (sprite.background_collider().has_circ_radius_px()) {
			auto radius = sprite.background_collider().circ_radius_px();
			current_rect = RectF::centered_around(collider_origin, radius * 2.0f, radius * 2.0f);
		}
	}
}
void State::BackgroundColliderMode::set() {
	// TODO
}
void State::BackgroundColliderMode::reset() {
	// TODO
}

State::ForegroundColliderMode::ForegroundColliderMode() {
	const auto& sprite = LEVEL.sheet_editor_state->selected_sprite();
	if (sprite.has_foreground_collider()) {
		auto collider_origin = VecF{sprite.center_offset_px()} + VecF{sprite.foreground_collider().center_offset_px()};
		if (sprite.foreground_collider().has_rect_dims_px()) {
			current_rect = RectF{collider_origin, VecF{sprite.foreground_collider().rect_dims_px()}};
		} else if (sprite.foreground_collider().has_circ_radius_px()) {
			auto radius = sprite.foreground_collider().circ_radius_px();
			current_rect = RectF::centered_around(collider_origin, radius * 2.0f, radius * 2.0f);
		}
	}
}
void State::ForegroundColliderMode::set() {
	// TODO
}
void State::ForegroundColliderMode::reset() {
	// TODO
}

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
const pb::Sprite& m2::sedit::State::selected_sprite() const {
	const auto& sheets = sprite_sheets();
	for (const auto& sheet : sheets.sheets()) {
		for (const auto& sprite : sheet.sprites()) {
			if (sprite.type() == _selected_sprite_type) {
				return sprite;
			}
		}
	}
	throw M2ERROR("Sprite sheet does not contain selected sprite");
}

RectI m2::sedit::State::selected_sprite_rect() const {
	return RectI{selected_sprite().rect()};
}

VecF m2::sedit::State::selected_sprite_center() const {
	return RectF{selected_sprite_rect()}.center();
}

VecF m2::sedit::State::selected_sprite_origin() const {
	return selected_sprite_center() + VecF{selected_sprite().center_offset_px()};
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

				// Move God to center if rect is already selected
				LEVEL.player()->position = selected_sprite_center();

				// Break from two loops
				goto image_loader_loaded;
			}
		}
	}
	image_loader_loaded:
	return;
	// TODO
}

void m2::sedit::State::deactivate_mode() {
	mode = std::monostate{};
}

void m2::sedit::State::activate_foreground_companion_mode() {
	mode = ForegroundCompanionMode{};
}

void m2::sedit::State::activate_rect_mode() {
	mode = RectMode{};
}

void m2::sedit::State::activate_background_collider_mode() {
	mode = BackgroundColliderMode{};
}

void m2::sedit::State::activate_foreground_collider_mode() {
	mode = ForegroundColliderMode{};
}
