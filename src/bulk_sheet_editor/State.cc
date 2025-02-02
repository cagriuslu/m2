#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/math/RectF.h>
#include <m2/bulk_sheet_editor/State.h>
#include <m2/game/Selection.h>
#include <m2/protobuf/Detail.h>
#include <SDL2/SDL_image.h>

using namespace m2;

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	constexpr SDL_Color CONFIRMED_SELECTION_COLOR = {0, 255, 0, 160};
}  // namespace

expected<bulk_sheet_editor::State> bulk_sheet_editor::State::create(const std::filesystem::path& sprite_sheets_path) {
	// If path exists,
	if (not std::filesystem::exists(sprite_sheets_path)) {
		return make_unexpected("SpriteSheets not found");
	}

	// Check if the file is a valid pb::SpriteSheets
	if (auto msg = pb::json_file_to_message<pb::SpriteSheets>(sprite_sheets_path); !msg) {
		return make_unexpected(msg.error());
	}

	return bulk_sheet_editor::State{sprite_sheets_path};
}

bulk_sheet_editor::State::~State() { Events::disable_primary_selection(); }

pb::SpriteSheets bulk_sheet_editor::State::sprite_sheets() const {
	return *pb::json_file_to_message<pb::SpriteSheets>(_sprite_sheets_path);
}

std::optional<pb::SpriteSheet> bulk_sheet_editor::State::selected_sprite_sheet() const {
	auto spriteSheets = this->sprite_sheets();
	// To find the selected resource in the sheets, iterate over sheets
	for (const auto& spriteSheet : spriteSheets.sheets()) {
		if (spriteSheet.resource() == _selected_resource.first) {
			return spriteSheet;
		}
	}
	return std::nullopt;  // Resource not yet selected
}

void bulk_sheet_editor::State::select_resource(const std::string& resource) {
	const auto& spriteSheets = this->sprite_sheets();
	// To find the selected resource in the sheets, iterate over sheets
	for (const auto& spriteSheet : spriteSheets.sheets()) {
		if (spriteSheet.resource() == resource) {
			_selected_resource = std::make_pair(resource, spriteSheet.ppm());
			return;
		}
	}
	throw M2_ERROR("Selected resource is not found in SpriteSheets");
}

bool bulk_sheet_editor::State::select() {
	const auto& spriteSheets = this->sprite_sheets();
	// To find the selected resource in the sheets, iterate over sheets
	for (const auto& spriteSheet : spriteSheets.sheets()) {
		if (spriteSheet.resource() == _selected_resource.first) {
			if (spriteSheet.sprites().empty()) {
				LOG_ERROR("Selected sprite sheet has no sprites");
				return false;
			}

			// Load image
			const auto resourcePath = spriteSheet.resource();
			sdl::SurfaceUniquePtr surface(IMG_Load(resourcePath.c_str()));
			if (!surface) {
				LOG_ERROR("Unable to load image", resourcePath, IMG_GetError());
				return false;
			}
			_texture = sdl::TextureUniquePtr{SDL_CreateTextureFromSurface(M2_GAME.renderer, surface.get())};
			if (!_texture) {
				LOG_ERROR("Unable to create texture from surface", SDL_GetError());
				return false;
			}
			_textureDimensions = {surface->w, surface->h};
			_ppm = spriteSheet.ppm();

			// Enable selection
			Events::enable_primary_selection(M2_GAME.Dimensions().Game());

			return true;
		}
	}
	return false;
}

void bulk_sheet_editor::State::select_sprite(m2g::pb::SpriteType type) {
	const auto spriteSheet = *selected_sprite_sheet();
	for (const auto& sprite : spriteSheet.sprites()) {
		if (sprite.type() == type) {
			_selected_sprite = std::make_pair(type, RectI{sprite.regular().rect()});

			const auto sprite_name = pb::enum_name(type);
			M2_LEVEL.ShowMessage(sprite_name);
			return;
		}
	}
	throw M2_ERROR("Selected sprite has been removed from the SpriteSheet");
}

void bulk_sheet_editor::State::modify_selected_sprite(const std::function<void(pb::Sprite&)>& modifier) const {
	sheet_editor::modify_sprite_in_sheet(_sprite_sheets_path, _selected_sprite.first, modifier);
}

void bulk_sheet_editor::State::set_rect() {
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

void bulk_sheet_editor::State::reset() {
	modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_rect();
		sprite.mutable_regular()->clear_center_to_origin_vec_px();
	});
	select_sprite(_selected_sprite.first);  // Reset rect
	M2_GAME.events.reset_primary_selection();
}

void bulk_sheet_editor::State::Draw() const {
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

	// Draw texture
	const auto offset = VecF{-0.5f, -0.5f};
	const auto textureTopLeftOutputPosition = ScreenOriginToPositionVecPx(offset);
	const auto textureBottomRightOutputPosition = ScreenOriginToPositionVecPx(static_cast<VecF>(_textureDimensions) + offset);
	const SDL_Rect dstRect = {
			iround(textureTopLeftOutputPosition.x), iround(textureTopLeftOutputPosition.y),
			iround(textureBottomRightOutputPosition.x - textureTopLeftOutputPosition.x),
			iround(textureBottomRightOutputPosition.y - textureTopLeftOutputPosition.y)};
	SDL_RenderCopy(M2_GAME.renderer, _texture.get(), nullptr, &dstRect);
	// Draw pixel grid lines
	Graphic::DrawGridLines({127, 127, 255, 80});
	// Draw PPM grid lines
	Graphic::DrawGridLines({255, 255, 255, 255}, 0, _ppm);
	// Draw sheet boundaries
	Graphic::draw_vertical_line(-0.5f, {255, 0, 0, 255});
	Graphic::draw_horizontal_line(-0.5f, {255, 0, 0, 255});
	Graphic::draw_vertical_line(F(_textureDimensions.x) - 0.5f, {255, 0, 0, 255});
	Graphic::draw_horizontal_line(F(_textureDimensions.y) - 0.5f, {255, 0, 0, 255});
}
