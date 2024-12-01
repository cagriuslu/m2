#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/math/RectF.h>
#include <m2/bulk_sheet_editor/State.h>
#include <m2/game/Selection.h>
#include <m2/game/object/Line.h>
#include <m2/protobuf/Detail.h>

using namespace m2;

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	constexpr SDL_Color CONFIRMED_SELECTION_COLOR = {0, 255, 0, 160};
}  // namespace

expected<bsedit::State> bsedit::State::create(const std::filesystem::path& sprite_sheets_path) {
	// If path exists,
	if (not std::filesystem::exists(sprite_sheets_path)) {
		return make_unexpected("SpriteSheets not found");
	}

	// Check if the file is a valid pb::SpriteSheets
	if (auto msg = pb::json_file_to_message<pb::SpriteSheets>(sprite_sheets_path); !msg) {
		return make_unexpected(msg.error());
	}

	return bsedit::State{sprite_sheets_path};
}

bsedit::State::~State() { Events::disable_primary_selection(); }

pb::SpriteSheets bsedit::State::sprite_sheets() const {
	return *pb::json_file_to_message<pb::SpriteSheets>(_sprite_sheets_path);
}

std::optional<pb::SpriteSheet> bsedit::State::selected_sprite_sheet() const {
	auto sprite_sheets = this->sprite_sheets();
	// To find the selected resource in the sheets, iterate over sheets
	for (const auto& sprite_sheet : sprite_sheets.sheets()) {
		if (sprite_sheet.resource() == _selected_resource.first) {
			return sprite_sheet;
		}
	}
	return std::nullopt;  // Resource not yet selected
}

void bsedit::State::select_resource(const std::string& resource) {
	const auto& sprite_sheets = this->sprite_sheets();
	// To find the selected resource in the sheets, iterate over sheets
	for (const auto& sprite_sheet : sprite_sheets.sheets()) {
		if (sprite_sheet.resource() == resource) {
			_selected_resource = std::make_pair(resource, sprite_sheet.ppm());
			return;
		}
	}
	throw M2_ERROR("Selected resource is not found in SpriteSheets");
}

bool bsedit::State::select() {
	// Get rid of previously created pixels, lines, etc.
	M2_LEVEL.reset_bulk_sheet_editor();

	const auto& sprite_sheets = this->sprite_sheets();
	// To find the selected resource in the sheets, iterate over sheets
	for (const auto& sprite_sheet : sprite_sheets.sheets()) {
		if (sprite_sheet.resource() == _selected_resource.first) {
			if (sprite_sheet.sprites().empty()) {
				LOG_ERROR("Selected sprite sheet has no sprites");
				return false;
			}

			// Reload dynamic image loader with the resource
			auto image_loader = DynamicSpriteSheetLoader::create(sprite_sheet.resource(), sprite_sheet.ppm());
			if (!image_loader) {
				throw M2_ERROR("Failed to load the image: " + sprite_sheet.resource());
			}
			_dynamic_sprite_sheet_loader.emplace(std::move(*image_loader));
			M2_LEVEL.dynamic_grid_lines_loader.emplace(SDL_Color{127, 127, 255, 80});

			// Creates lines showing the boundaries of the sheet
			obj::create_vertical_line(-0.5f, SDL_Color{255, 0, 0, 255});
			obj::create_horizontal_line(-0.5f, SDL_Color{255, 0, 0, 255});
			const auto image_size = _dynamic_sprite_sheet_loader->image_size();
			obj::create_vertical_line(F(image_size.x / sprite_sheet.ppm()) - 0.5f, SDL_Color{255, 0, 0, 255});
			obj::create_horizontal_line(F(image_size.y / sprite_sheet.ppm()) - 0.5f, SDL_Color{255, 0, 0, 255});

			// Enable selection
			Events::enable_primary_selection(M2_GAME.dimensions().game);

			return true;
		}
	}
	return false;
}

void bsedit::State::select_sprite(m2g::pb::SpriteType type) {
	const auto sprite_sheet = *selected_sprite_sheet();
	for (const auto& sprite : sprite_sheet.sprites()) {
		if (sprite.type() == type) {
			_selected_sprite = std::make_pair(type, RectI{sprite.regular().rect()});

			const auto sprite_name = pb::enum_name(type);
			M2_LEVEL.display_message(sprite_name);
			return;
		}
	}
	throw M2_ERROR("Selected sprite has been removed from the SpriteSheet");
}

void bsedit::State::modify_selected_sprite(const std::function<void(pb::Sprite&)>& modifier) const {
	sedit::modify_sprite_in_sheet(_sprite_sheets_path, _selected_sprite.first, modifier);
}

void bsedit::State::set_rect() {
	auto selection_results = SelectionResult{M2_GAME.events};
	// If rect is selected
	if (selection_results.is_primary_selection_finished()) {
		LOG_DEBUG("Primary selection");
		auto positions = selection_results.primary_int_selection_position_m();
		auto rect = RectI::from_corners(positions->first, positions->second);  // wrt sprite coordinates
		modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_rect()->set_x(rect.x * _selected_resource.second);
			sprite.mutable_regular()->mutable_rect()->set_y(rect.y * _selected_resource.second);
			sprite.mutable_regular()->mutable_rect()->set_w(rect.w * _selected_resource.second);
			sprite.mutable_regular()->mutable_rect()->set_h(rect.h * _selected_resource.second);
		});
		select_sprite(_selected_sprite.first);  // Reset rect
		M2_GAME.events.reset_primary_selection();
	}
}

void bsedit::State::reset() {
	modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_rect();
		sprite.mutable_regular()->clear_center_to_origin_vec_px();
	});
	select_sprite(_selected_sprite.first);  // Reset rect
	M2_GAME.events.reset_primary_selection();
}

void bsedit::State::on_draw() const {
	// Draw selection
	if (auto positions = SelectionResult{M2_GAME.events}.primary_cell_selection_position_m(); positions) {
		Graphic::color_rect(RectF::from_corners(positions->first, positions->second), SELECTION_COLOR);
	}
	// Draw currectly selected sprite's rect
	if (_selected_sprite.second) {
		auto world_coordinates_m = RectF{
		    F(_selected_sprite.second.x) / F(_selected_resource.second),
		    F(_selected_sprite.second.y) / F(_selected_resource.second),
		    F(_selected_sprite.second.w) / F(_selected_resource.second),
		    F(_selected_sprite.second.h) / F(_selected_resource.second)};
		Graphic::color_rect(world_coordinates_m.shift({-0.5f, -0.5f}), CONFIRMED_SELECTION_COLOR);
	}
}
