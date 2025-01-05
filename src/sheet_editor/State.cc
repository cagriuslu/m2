#include <m2/sheet_editor/State.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/game/Selection.h>
#include <m2/game/object/Line.h>
#include <m2/protobuf/Detail.h>

using namespace m2;
using namespace m2::sedit;

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	constexpr SDL_Color YELLOW_SELECTION_COLOR = {127, 127, 0, 180};
	constexpr SDL_Color RED_SELECTION_COLOR = {255, 0, 0, 180};
	constexpr SDL_Color CONFIRMED_SELECTION_COLOR = {0, 255, 0, 80};
	constexpr SDL_Color CROSS_COLOR = {0, 127, 255, 255};
	constexpr SDL_Color CONFIRMED_CROSS_COLOR = {0, 255, 0, 255};
}  // namespace

State::ForegroundCompanionMode::ForegroundCompanionMode() {
	const auto& sprite = std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite();
	if (sprite.has_regular() && sprite.regular().foreground_companion_rects_size()) {
		// Iterate over rects
		for (const auto& rect : sprite.regular().foreground_companion_rects()) {
			current_rects.emplace_back(rect);
		}
		// Set center
		current_center = VecF{sprite.regular().foreground_companion_center_to_origin_vec_px()};
	}
	// Enable selection
	Events::enable_primary_selection(RectI{M2_GAME.Dimensions().Game()});
}
State::ForegroundCompanionMode::~ForegroundCompanionMode() { Events::disable_primary_selection(); }
void State::ForegroundCompanionMode::on_draw() const {
	// Draw selection
	if (auto positions = SelectionResult{M2_GAME.events}.primary_cell_selection_position_m(); positions) {
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
		auto sprite_center = std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center();
		Graphic::draw_cross(sprite_center + *current_center, CONFIRMED_CROSS_COLOR);
	}
}
void State::ForegroundCompanionMode::add_rect() {
	auto selection_results = SelectionResult{M2_GAME.events};
	// If rect is selected
	if (selection_results.is_primary_selection_finished()) {
		auto positions = selection_results.primary_int_selection_position_m();
		auto rect = RectI::from_corners(positions->first, positions->second);  // wrt sprite coordinates
		std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
			auto* new_rect = sprite.mutable_regular()->add_foreground_companion_rects();
			new_rect->set_x(rect.x);
			new_rect->set_y(rect.y);
			new_rect->set_w(rect.w);
			new_rect->set_h(rect.h);
		});
		current_rects.emplace_back(rect);
		M2_GAME.events.reset_primary_selection();
	}
}
void State::ForegroundCompanionMode::set_center() {
	// Store center selection
	if (secondary_selection_position) {
		auto center_offset = *secondary_selection_position -
		    std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center();  // new offset from sprite center
		std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_foreground_companion_center_to_origin_vec_px()->set_x(center_offset.x);
			sprite.mutable_regular()->mutable_foreground_companion_center_to_origin_vec_px()->set_y(center_offset.y);
		});
		current_center = center_offset;
		secondary_selection_position = std::nullopt;
	}
}
void State::ForegroundCompanionMode::reset() {
	std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_foreground_companion_center_to_origin_vec_px();
		sprite.mutable_regular()->clear_foreground_companion_rects();
	});
	current_rects.clear();
	current_center = std::nullopt;
	M2_GAME.events.reset_primary_selection();
	secondary_selection_position = std::nullopt;
}

State::RectMode::RectMode() {
	const auto& sprite = std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite();
	// Set rect
	current_rect = RectI{sprite.regular().rect()};
	// Set center
	current_center = VecF{sprite.regular().center_to_origin_vec_px()};
	// Enable selection
	Events::enable_primary_selection(M2_GAME.Dimensions().Game());
}
State::RectMode::~RectMode() { Events::disable_primary_selection(); }
void State::RectMode::on_draw() const {
	// Draw selection
	if (const auto positions = SelectionResult{M2_GAME.events}.primary_cell_selection_position_m(); positions) {
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
		auto sprite_center = std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center();
		Graphic::draw_cross(sprite_center + *current_center, CONFIRMED_CROSS_COLOR);
	}
}
void State::RectMode::set_rect() {
	auto selection_results = SelectionResult{M2_GAME.events};
	// If rect is selected
	if (selection_results.is_primary_selection_finished()) {
		LOG_DEBUG("Primary selection");
		auto positions = selection_results.primary_int_selection_position_m();
		auto rect = RectI::from_corners(positions->first, positions->second);  // wrt sprite coordinates
		std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_rect()->set_x(rect.x);
			sprite.mutable_regular()->mutable_rect()->set_y(rect.y);
			sprite.mutable_regular()->mutable_rect()->set_w(rect.w);
			sprite.mutable_regular()->mutable_rect()->set_h(rect.h);
		});
		current_rect = rect;
		M2_GAME.events.reset_primary_selection();
	}
}
void State::RectMode::set_center() {
	// Store center selection
	if (secondary_selection_position) {
		auto center_offset = *secondary_selection_position -
		    std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center();  // new offset from sprite center
		std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_center_to_origin_vec_px()->set_x(center_offset.x);
			sprite.mutable_regular()->mutable_center_to_origin_vec_px()->set_y(center_offset.y);
		});
		current_center = center_offset;
		secondary_selection_position = std::nullopt;
	}
}
void State::RectMode::reset() {
	std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_rect();
		sprite.mutable_regular()->clear_center_to_origin_vec_px();
	});
	current_rect = std::nullopt;
	current_center = std::nullopt;
	M2_GAME.events.reset_primary_selection();
	secondary_selection_position = std::nullopt;
}

State::BackgroundColliderMode::BackgroundColliderMode() {
	const auto& sprite = std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite();
	if (sprite.regular().has_background_collider()) {
		auto collider_origin =
		    VecF{sprite.regular().center_to_origin_vec_px()} + VecF{sprite.regular().background_collider().origin_to_origin_vec_px()};
		if (sprite.regular().background_collider().has_rect_dims_px()) {
			current_rect = RectF::centered_around(collider_origin, VecF{sprite.regular().background_collider().rect_dims_px()});
		} else if (sprite.regular().background_collider().has_circ_radius_px()) {
			auto radius = sprite.regular().background_collider().circ_radius_px();
			current_circ = CircF{collider_origin, radius};
		}
	}
	// Enable selection
	Events::enable_primary_selection(RectI{M2_GAME.Dimensions().Game()});
	Events::enable_secondary_selection(RectI{M2_GAME.Dimensions().Game()});
}
State::BackgroundColliderMode::~BackgroundColliderMode() {
	Events::disable_primary_selection();
	Events::disable_secondary_selection();
}
void State::BackgroundColliderMode::on_draw() const {
	// Draw rect selection
	if (auto positions = SelectionResult{M2_GAME.events}.primary_halfcell_selection_position_m(); positions) {
		Graphic::color_rect(RectF::from_corners(positions->first, positions->second), YELLOW_SELECTION_COLOR);
	}
	// Draw circ selection
	if (auto positions = SelectionResult{M2_GAME.events}.secondary_halfcell_selection_position_m(); positions) {
		Graphic::color_disk(positions->first, positions->first.distance(positions->second), RED_SELECTION_COLOR);
	}

	if (current_rect) {
		auto sprite_center = std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center();
		auto rect = current_rect->shift(sprite_center);
		Graphic::color_rect(rect, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(rect.center(), CONFIRMED_CROSS_COLOR);
	}
	if (current_circ) {
		// Find location of the circle
		auto sprite_center = std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center();
		auto circ = CircF{current_circ->center + sprite_center, current_circ->r};
		Graphic::color_disk(circ.center, circ.r, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(circ.center, CONFIRMED_CROSS_COLOR);
	}
}
void State::BackgroundColliderMode::set() {
	auto selection_results = SelectionResult{M2_GAME.events};
	// If rect is selected
	if (selection_results.is_primary_selection_finished()) {
		LOG_DEBUG("Primary selection");
		auto positions = selection_results.primary_halfcell_selection_position_m();
		auto rect = RectF::from_corners(positions->first, positions->second);  // wrt sprite coordinates
		auto origin_offset = rect.center() -
		    std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_origin();  // new offset from sprite origin
		auto dims = VecF{rect.w, rect.h};  // new dims
		std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_background_collider()->mutable_origin_to_origin_vec_px()->set_x(origin_offset.x);
			sprite.mutable_regular()->mutable_background_collider()->mutable_origin_to_origin_vec_px()->set_y(origin_offset.y);
			sprite.mutable_regular()->mutable_background_collider()->mutable_rect_dims_px()->set_w(dims.x);
			sprite.mutable_regular()->mutable_background_collider()->mutable_rect_dims_px()->set_h(dims.y);
		});
		current_rect = rect.shift_origin(std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center());
		current_circ = std::nullopt;
		M2_GAME.events.reset_primary_selection();
	} else if (selection_results.is_secondary_selection_finished()) {
		LOG_DEBUG("Secondary selection");
		// If circ is selected
		auto positions = selection_results.secondary_halfcell_selection_position_m();
		auto center = positions->first;
		auto radius = positions->first.distance(positions->second);
		auto origin_offset = center -
		    std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_origin();  // new offset from sprite origin
		std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_background_collider()->mutable_origin_to_origin_vec_px()->set_x(origin_offset.x);
			sprite.mutable_regular()->mutable_background_collider()->mutable_origin_to_origin_vec_px()->set_y(origin_offset.y);
			sprite.mutable_regular()->mutable_background_collider()->set_circ_radius_px(radius);
		});
		current_rect = std::nullopt;
		current_circ = CircF{center - std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center(), radius};
		M2_GAME.events.reset_secondary_selection();
	}
}
void State::BackgroundColliderMode::reset() {
	std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_background_collider();
	});
	current_rect = std::nullopt;
	current_circ = std::nullopt;
	M2_GAME.events.reset_primary_selection();
	M2_GAME.events.reset_secondary_selection();
}

State::ForegroundColliderMode::ForegroundColliderMode() {
	const auto& sprite = std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite();
	if (sprite.regular().has_foreground_collider()) {
		auto collider_origin =
		    VecF{sprite.regular().center_to_origin_vec_px()} + VecF{sprite.regular().foreground_collider().origin_to_origin_vec_px()};
		if (sprite.regular().foreground_collider().has_rect_dims_px()) {
			current_rect = RectF::centered_around(collider_origin, VecF{sprite.regular().foreground_collider().rect_dims_px()});
		} else if (sprite.regular().foreground_collider().has_circ_radius_px()) {
			auto radius = sprite.regular().foreground_collider().circ_radius_px();
			current_circ = CircF{collider_origin, radius};
		}
	}
	// Enable selection
	Events::enable_primary_selection(RectI{M2_GAME.Dimensions().Game()});
	Events::enable_secondary_selection(RectI{M2_GAME.Dimensions().Game()});
}
State::ForegroundColliderMode::~ForegroundColliderMode() {
	Events::disable_primary_selection();
	Events::disable_secondary_selection();
}
void State::ForegroundColliderMode::on_draw() const {
	// Draw rect selection
	if (auto positions = SelectionResult{M2_GAME.events}.primary_halfcell_selection_position_m(); positions) {
		Graphic::color_rect(RectF::from_corners(positions->first, positions->second), YELLOW_SELECTION_COLOR);
	}
	// Draw circ selection
	if (auto positions = SelectionResult{M2_GAME.events}.secondary_halfcell_selection_position_m(); positions) {
		Graphic::color_disk(positions->first, positions->first.distance(positions->second), RED_SELECTION_COLOR);
	}

	if (current_rect) {
		auto sprite_center = std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center();
		auto rect = current_rect->shift(sprite_center);
		Graphic::color_rect(rect, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(rect.center(), CONFIRMED_CROSS_COLOR);
	}
	if (current_circ) {
		auto sprite_center = std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center();
		auto circ = CircF{current_circ->center + sprite_center, current_circ->r};
		Graphic::color_disk(circ.center, circ.r, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(circ.center, CONFIRMED_CROSS_COLOR);
	}
}
void State::ForegroundColliderMode::set() {
	auto selection_results = SelectionResult{M2_GAME.events};
	// If rect is selected
	if (selection_results.is_primary_selection_finished()) {
		LOG_DEBUG("Primary selection");
		auto positions = selection_results.primary_halfcell_selection_position_m();
		auto rect = RectF::from_corners(positions->first, positions->second);  // wrt sprite coordinates
		auto origin_offset = rect.center() -
		    std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_origin();  // new offset from sprite origin
		auto dims = VecF{rect.w, rect.h};  // new dims
		std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_origin_to_origin_vec_px()->set_x(origin_offset.x);
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_origin_to_origin_vec_px()->set_y(origin_offset.y);
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_rect_dims_px()->set_w(dims.x);
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_rect_dims_px()->set_h(dims.y);
		});
		current_rect = rect.shift_origin(std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center());
		current_circ = std::nullopt;
		M2_GAME.events.reset_primary_selection();
	} else if (selection_results.is_secondary_selection_finished()) {
		LOG_DEBUG("Secondary selection");
		// If circ is selected
		auto positions = selection_results.secondary_halfcell_selection_position_m();
		auto center = positions->first;
		auto radius = positions->first.distance(positions->second);
		auto origin_offset = center -
		    std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_origin();  // new offset from sprite origin
		std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_origin_to_origin_vec_px()->set_x(origin_offset.x);
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_origin_to_origin_vec_px()->set_y(origin_offset.y);
			sprite.mutable_regular()->mutable_foreground_collider()->set_circ_radius_px(radius);
		});
		current_rect = std::nullopt;
		current_circ = CircF{center - std::get<sedit::State>(M2_LEVEL.type_state).selected_sprite_center(), radius};
		M2_GAME.events.reset_secondary_selection();
	}
}
void State::ForegroundColliderMode::reset() {
	std::get<sedit::State>(M2_LEVEL.type_state).modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_foreground_collider();
	});
	current_rect = std::nullopt;
	current_circ = std::nullopt;
	M2_GAME.events.reset_primary_selection();
	M2_GAME.events.reset_secondary_selection();
}

expected<m2::sedit::State> m2::sedit::State::create(const std::filesystem::path& path) {
	// If path exists,
	if (std::filesystem::exists(path)) {
		// Check if the file is a valid pb::SpriteSheets
		if (auto msg = pb::json_file_to_message<pb::SpriteSheets>(path); !msg) {
			return make_unexpected(msg.error());
		}
	}
	return State{path};
}

const m2::pb::SpriteSheets& m2::sedit::State::sprite_sheets() const {
	// If path exists,
	if (std::filesystem::exists(_path)) {
		// Check if the file is a valid pb::SpriteSheets
		if (auto msg = pb::json_file_to_message<pb::SpriteSheets>(_path); msg) {
			_sprite_sheets = *msg;
		} else {
			throw M2_ERROR("File is not a valid m2::pb::SpriteSheets: " + _path.string());
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
	throw M2_ERROR("Sprite sheet does not contain selected sprite");
}

void m2::sedit::State::modify_selected_sprite(const std::function<void(pb::Sprite&)>& modifier) {
	modify_sprite_in_sheet(_path, _selected_sprite_type, modifier);
}

RectI m2::sedit::State::selected_sprite_rect() const { return RectI{selected_sprite().regular().rect()}; }

VecF m2::sedit::State::selected_sprite_center() const {
	// Rect needs to be shifted to fit into cells
	auto rect = RectF{selected_sprite_rect()}.shift({-0.5f, -0.5f});
	return rect.center();
}

VecF m2::sedit::State::selected_sprite_origin() const {
	return selected_sprite_center() + VecF{selected_sprite().regular().center_to_origin_vec_px()};
}

void m2::sedit::State::set_sprite_type(m2g::pb::SpriteType sprite_type) { _selected_sprite_type = sprite_type; }

void m2::sedit::State::select() {
	// Get rid of previously created pixels, lines, etc.
	M2_LEVEL.ResetSheetEditor();

	const auto& spriteSheets = this->sprite_sheets();
	// Reload dynamic image loader with the resource
	// To find sprite in the sheet, iterate over sheets
	for (const auto& spriteSheet : spriteSheets.sheets()) {
		// Iterate over sprites
		for (const auto& sprite : spriteSheet.sprites()) {
			if (sprite.type() == _selected_sprite_type) {
				auto image_loader = DynamicImageLoader::create(spriteSheet.resource());
				if (!image_loader) {
					throw M2_ERROR("Failed to load the image: " + spriteSheet.resource());
				}
				_dynamic_image_loader.emplace(std::move(*image_loader));
				M2_LEVEL.dynamic_grid_lines_loader.emplace(SDL_Color{127, 127, 255, 80});
				M2_LEVEL.dynamic_sheet_grid_lines_loader.emplace(SDL_Color{255, 255, 255, 80}, spriteSheet.ppm());

				// Creates lines showing the boundaries of the sheet
				obj::create_vertical_line(-0.5f, SDL_Color{255, 0, 0, 255});
				obj::create_horizontal_line(-0.5f, SDL_Color{255, 0, 0, 255});
				const auto image_size = _dynamic_image_loader->image_size();
				obj::create_vertical_line(F(image_size.x) - 0.5f, SDL_Color{255, 0, 0, 255});
				obj::create_horizontal_line(F(image_size.y) - 0.5f, SDL_Color{255, 0, 0, 255});

				// Move God to center if rect is already selected
				M2_PLAYER.position = selected_sprite_center();

				return;
			}
		}
	}
}

void m2::sedit::State::deactivate_mode() { mode.emplace<std::monostate>(); }

void m2::sedit::State::activate_foreground_companion_mode() { mode.emplace<ForegroundCompanionMode>(); }

void m2::sedit::State::activate_rect_mode() { mode.emplace<RectMode>(); }

void m2::sedit::State::activate_background_collider_mode() { mode.emplace<BackgroundColliderMode>(); }

void m2::sedit::State::activate_foreground_collider_mode() { mode.emplace<ForegroundColliderMode>(); }

void m2::sedit::modify_sprite_in_sheet(
    const std::filesystem::path& path, m2g::pb::SpriteType type, const std::function<void(pb::Sprite&)>& modifier) {
	// If path exists,
	if (std::filesystem::exists(path)) {
		// Check if the file is a valid pb::SpriteSheets
		if (auto spriteSheets = pb::json_file_to_message<pb::SpriteSheets>(path); spriteSheets) {
			for (int i = 0; i < spriteSheets->sheets_size(); ++i) {
				auto* mutable_sheet = spriteSheets->mutable_sheets(i);
				for (int j = 0; j < mutable_sheet->sprites_size(); ++j) {
					auto* mutable_sprite = mutable_sheet->mutable_sprites(j);
					if (mutable_sprite->type() == type) {
						modifier(*mutable_sprite);
						message_to_json_file(*spriteSheets, path);
						return;
					}
				}
			}
			throw M2_ERROR("Sprite not found in SpriteSheets");
		} else {
			throw M2_ERROR("File is not a valid m2::pb::SpriteSheets: " + path.string());
		}
	} else {
		throw M2_ERROR("Can't modify nonexistent file");
	}
}
