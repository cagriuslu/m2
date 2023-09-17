#include <m2/sheet_editor/State.h>
#include <m2/protobuf/Detail.h>
#include <m2/Game.h>
#include <m2/game/Selection.h>

using namespace m2;
using namespace m2::sedit;

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	constexpr SDL_Color YELLOW_SELECTION_COLOR = {127, 127, 0, 180};
	constexpr SDL_Color RED_SELECTION_COLOR = {255, 0, 0, 180};
	constexpr SDL_Color CONFIRMED_SELECTION_COLOR = {0, 255, 0, 80};
	constexpr SDL_Color CROSS_COLOR = {0, 127, 255, 255};
	constexpr SDL_Color CONFIRMED_CROSS_COLOR = {0, 255, 0, 255};
}

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
	// Enable selection
	Events::enable_primary_selection(RectI{GAME.dimensions().game});
}
State::ForegroundCompanionMode::~ForegroundCompanionMode() {
	Events::disable_primary_selection();
}
void State::ForegroundCompanionMode::on_draw() const {
	// Draw selection
	if (auto positions = SelectionResult{GAME.events}.primary_cell_selection_position_m(); positions) {
		Graphic::color_rect(RectF::from_corners(positions->first, positions->second), SELECTION_COLOR);
	}
	// Draw center selection
	if (secondary_selection_position) {
		Graphic::draw_cross(*secondary_selection_position, CROSS_COLOR);
	}

	for (const auto& rect : current_rects) {
		Graphic::color_rect(RectF{rect}.shift({-0.5f, -0.5f}), CONFIRMED_SELECTION_COLOR);
	}
	if (current_center) {
		auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
		Graphic::draw_cross(sprite_center + *current_center, CONFIRMED_CROSS_COLOR);
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
	// Enable selection
	Events::enable_primary_selection(RectI{GAME.dimensions().game});
}
State::RectMode::~RectMode() {
	Events::disable_primary_selection();
}
void State::RectMode::on_draw() const {
	// Draw selection
	if (auto positions = SelectionResult{GAME.events}.primary_cell_selection_position_m(); positions) {
		Graphic::color_rect(RectF::from_corners(positions->first, positions->second), SELECTION_COLOR);
	}
	// Draw center selection
	if (secondary_selection_position) {
		Graphic::draw_cross(*secondary_selection_position, CROSS_COLOR);
	}

	if (current_rect) {
		Graphic::color_rect(RectF{*current_rect}.shift({-0.5f, -0.5f}), CONFIRMED_SELECTION_COLOR);
	}
	if (current_center) {
		auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
		Graphic::draw_cross(sprite_center + *current_center, CONFIRMED_CROSS_COLOR);
	}
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
		auto collider_origin = VecF{sprite.center_offset_px()} + VecF{sprite.background_collider().origin_offset_px()};
		if (sprite.background_collider().has_rect_dims_px()) {
			current_rect = RectF{collider_origin, VecF{sprite.background_collider().rect_dims_px()}};
		} else if (sprite.background_collider().has_circ_radius_px()) {
			auto radius = sprite.background_collider().circ_radius_px();
			current_circ = CircF{collider_origin, radius};
		}
	}
	// Enable selection
	Events::enable_primary_selection(RectI{GAME.dimensions().game});
	Events::enable_secondary_selection(RectI{GAME.dimensions().game});
}
State::BackgroundColliderMode::~BackgroundColliderMode() {
	Events::disable_primary_selection();
	Events::disable_secondary_selection();
}
void State::BackgroundColliderMode::on_draw() const {
	// Draw rect selection
	if (auto positions = SelectionResult{GAME.events}.primary_halfcell_selection_position_m(); positions) {
		Graphic::color_rect(RectF::from_corners(positions->first, positions->second), YELLOW_SELECTION_COLOR);
	}
	// Draw circ selection
	if (auto positions = SelectionResult{GAME.events}.secondary_halfcell_selection_position_m(); positions) {
		Graphic::color_disk(positions->first, positions->first.distance(positions->second), RED_SELECTION_COLOR);
	}

	if (current_rect) {
		auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
		auto rect = current_rect->shift(sprite_center);
		Graphic::color_rect(rect, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(rect.center(), CONFIRMED_CROSS_COLOR);
	}
	if (current_circ) {
		// Find location of the circle
		auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
		auto circ = CircF{current_circ->center + sprite_center, current_circ->r};
		Graphic::color_disk(circ.center, circ.r, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(circ.center, CONFIRMED_CROSS_COLOR);
	}
}
void State::BackgroundColliderMode::set() {
	auto selection_results = SelectionResult{GAME.events};
	// If rect is selected
	if (selection_results.is_primary_selection_finished()) {
		LOG_DEBUG("Primary selection");
		auto positions = selection_results.primary_halfcell_selection_position_m();
		auto rect = RectF::from_corners(positions->first, positions->second); // wrt sprite coordinates
		auto origin_offset = rect.center() - LEVEL.sheet_editor_state->selected_sprite_origin(); // new offset from sprite origin
		auto dims = VecF{rect.w, rect.h}; // new dims
		LEVEL.sheet_editor_state->modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_background_collider()->mutable_origin_offset_px()->set_x(origin_offset.x);
			sprite.mutable_background_collider()->mutable_origin_offset_px()->set_y(origin_offset.y);
			sprite.mutable_background_collider()->mutable_rect_dims_px()->set_w(dims.x);
			sprite.mutable_background_collider()->mutable_rect_dims_px()->set_h(dims.y);
		});
		current_rect = rect.shift_origin(LEVEL.sheet_editor_state->selected_sprite_center());
		current_circ = std::nullopt;
		GAME.events.reset_primary_selection();
	} else if (selection_results.is_secondary_selection_finished()) {
		LOG_DEBUG("Secondary selection");
		// If circ is selected
		auto positions = selection_results.secondary_halfcell_selection_position_m();
		auto center = positions->first;
		auto radius = positions->first.distance(positions->second);
		auto origin_offset = center - LEVEL.sheet_editor_state->selected_sprite_origin(); // new offset from sprite origin
		LEVEL.sheet_editor_state->modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_background_collider()->mutable_origin_offset_px()->set_x(origin_offset.x);
			sprite.mutable_background_collider()->mutable_origin_offset_px()->set_y(origin_offset.y);
			sprite.mutable_background_collider()->set_circ_radius_px(radius);
		});
		current_rect = std::nullopt;
		current_circ = CircF{center - LEVEL.sheet_editor_state->selected_sprite_center(), radius};
		GAME.events.reset_secondary_selection();
	}
}
void State::BackgroundColliderMode::reset() {
	LEVEL.sheet_editor_state->modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.clear_background_collider();
	});
	current_rect = std::nullopt;
	current_circ = std::nullopt;
	GAME.events.reset_primary_selection();
	GAME.events.reset_secondary_selection();
}

State::ForegroundColliderMode::ForegroundColliderMode() {
	const auto& sprite = LEVEL.sheet_editor_state->selected_sprite();
	if (sprite.has_foreground_collider()) {
		auto collider_origin = VecF{sprite.center_offset_px()} + VecF{sprite.foreground_collider().origin_offset_px()};
		if (sprite.foreground_collider().has_rect_dims_px()) {
			current_rect = RectF{collider_origin, VecF{sprite.foreground_collider().rect_dims_px()}};
		} else if (sprite.foreground_collider().has_circ_radius_px()) {
			auto radius = sprite.foreground_collider().circ_radius_px();
			current_circ = CircF{collider_origin, radius};
		}
	}
	// Enable selection
	Events::enable_primary_selection(RectI{GAME.dimensions().game});
	Events::enable_secondary_selection(RectI{GAME.dimensions().game});
}
State::ForegroundColliderMode::~ForegroundColliderMode() {
	Events::disable_primary_selection();
	Events::disable_secondary_selection();
}
void State::ForegroundColliderMode::on_draw() const {
	// Draw rect selection
	if (auto positions = SelectionResult{GAME.events}.primary_halfcell_selection_position_m(); positions) {
		Graphic::color_rect(RectF::from_corners(positions->first, positions->second), YELLOW_SELECTION_COLOR);
	}
	// Draw circ selection
	if (auto positions = SelectionResult{GAME.events}.secondary_halfcell_selection_position_m(); positions) {
		Graphic::color_disk(positions->first, positions->first.distance(positions->second), RED_SELECTION_COLOR);
	}

	if (current_rect) {
		auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
		auto rect = current_rect->shift(sprite_center);
		Graphic::color_rect(rect, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(rect.center(), CONFIRMED_CROSS_COLOR);
	}
	if (current_circ) {
		auto sprite_center = LEVEL.sheet_editor_state->selected_sprite_center();
		auto circ = CircF{current_circ->center + sprite_center, current_circ->r};
		Graphic::color_disk(circ.center, circ.r, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(circ.center, CONFIRMED_CROSS_COLOR);
	}
}
void State::ForegroundColliderMode::set() {
	auto selection_results = SelectionResult{GAME.events};
	// If rect is selected
	if (selection_results.is_primary_selection_finished()) {
		LOG_DEBUG("Primary selection");
		auto positions = selection_results.primary_halfcell_selection_position_m();
		auto rect = RectF::from_corners(positions->first, positions->second); // wrt sprite coordinates
		auto origin_offset = rect.center() - LEVEL.sheet_editor_state->selected_sprite_origin(); // new offset from sprite origin
		auto dims = VecF{rect.w, rect.h}; // new dims
		LEVEL.sheet_editor_state->modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_foreground_collider()->mutable_origin_offset_px()->set_x(origin_offset.x);
			sprite.mutable_foreground_collider()->mutable_origin_offset_px()->set_y(origin_offset.y);
			sprite.mutable_foreground_collider()->mutable_rect_dims_px()->set_w(dims.x);
			sprite.mutable_foreground_collider()->mutable_rect_dims_px()->set_h(dims.y);
		});
		current_rect = rect.shift_origin(LEVEL.sheet_editor_state->selected_sprite_center());
		current_circ = std::nullopt;
		GAME.events.reset_primary_selection();
	} else if (selection_results.is_secondary_selection_finished()) {
		LOG_DEBUG("Secondary selection");
		// If circ is selected
		auto positions = selection_results.secondary_halfcell_selection_position_m();
		auto center = positions->first;
		auto radius = positions->first.distance(positions->second);
		auto origin_offset = center - LEVEL.sheet_editor_state->selected_sprite_origin(); // new offset from sprite origin
		LEVEL.sheet_editor_state->modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_foreground_collider()->mutable_origin_offset_px()->set_x(origin_offset.x);
			sprite.mutable_foreground_collider()->mutable_origin_offset_px()->set_y(origin_offset.y);
			sprite.mutable_foreground_collider()->set_circ_radius_px(radius);
		});
		current_rect = std::nullopt;
		current_circ = CircF{center - LEVEL.sheet_editor_state->selected_sprite_center(), radius};
		GAME.events.reset_secondary_selection();
	}
}
void State::ForegroundColliderMode::reset() {
	LEVEL.sheet_editor_state->modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.clear_foreground_collider();
	});
	current_rect = std::nullopt;
	current_circ = std::nullopt;
	GAME.events.reset_primary_selection();
	GAME.events.reset_secondary_selection();
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

void m2::sedit::State::modify_selected_sprite(std::function<void(pb::Sprite&)> modifier) {
	// If path exists,
	if (std::filesystem::exists(_path)) {
		// Check if the file is a valid pb::SpriteSheets
		if (auto sprite_sheets = protobuf::json_file_to_message<pb::SpriteSheets>(_path); sprite_sheets) {
			for (int i = 0; i < sprite_sheets->sheets_size(); ++i) {
				auto* mutable_sheet = sprite_sheets->mutable_sheets(i);
				for (int j = 0; j < mutable_sheet->sprites_size(); ++j) {
					auto* mutable_sprite = mutable_sheet->mutable_sprites(j);
					if (mutable_sprite->type() == _selected_sprite_type) {
						modifier(*mutable_sprite);
						protobuf::message_to_json_file(*sprite_sheets, _path);
						return;
					}
				}
			}
			throw M2ERROR("Sprite not found in SpriteSheets");
		} else {
			throw M2ERROR("File is not a valid m2::pb::SpriteSheets: " + _path.string());
		}
	} else {
		throw M2ERROR("Can't modify nonexistent file");
	}
}

RectI m2::sedit::State::selected_sprite_rect() const {
	return RectI{selected_sprite().rect()};
}

VecF m2::sedit::State::selected_sprite_center() const {
	// Rect needs to be shifted to fit into cells
	auto rect = RectF{selected_sprite_rect()}.shift({-0.5f, -0.5f});
	return rect.center();
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
	mode.emplace<std::monostate>();
}

void m2::sedit::State::activate_foreground_companion_mode() {
	mode.emplace<ForegroundCompanionMode>();
}

void m2::sedit::State::activate_rect_mode() {
	mode.emplace<RectMode>();
}

void m2::sedit::State::activate_background_collider_mode() {
	mode.emplace<BackgroundColliderMode>();
}

void m2::sedit::State::activate_foreground_collider_mode() {
	mode.emplace<ForegroundColliderMode>();
}
