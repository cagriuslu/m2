#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/math/RectF.h>
#include <m2/bulk_sheet_editor/State.h>
#include <m2/game/Selection.h>
#include <m2/protobuf/Detail.h>
#include <m2/ui/widget/TextSelection.h>
#include <SDL2/SDL_image.h>

using namespace m2;

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	constexpr SDL_Color CONFIRMED_SELECTION_COLOR = {0, 255, 0, 80};
}  // namespace

expected<bulk_sheet_editor::State> bulk_sheet_editor::State::Create(const std::filesystem::path& sprite_sheets_path) {
	// If path exists,
	if (not std::filesystem::exists(sprite_sheets_path)) {
		return make_unexpected("SpriteSheets not found");
	}

	// Check if the file is a valid pb::SpriteSheets
	if (auto msg = pb::json_file_to_message<pb::SpriteSheets>(sprite_sheets_path); !msg) {
		return make_unexpected(msg.error());
	}

	return State{sprite_sheets_path};
}

bulk_sheet_editor::State::~State() { M2_LEVEL.DisablePrimarySelection(); }

pb::SpriteSheets bulk_sheet_editor::State::ReadSpriteSheetsFromFile() const {
	return *pb::json_file_to_message<pb::SpriteSheets>(_sprite_sheets_path);
}

std::optional<pb::SpriteSheet> bulk_sheet_editor::State::SelectResource(const std::string& resource) {
	const auto& spriteSheets = ReadSpriteSheetsFromFile();
	// To find the selected resource in the sheets, iterate over sheets
	for (const auto& spriteSheet : spriteSheets.sheets()) {
		if (spriteSheet.resource() == resource) {
			if (spriteSheet.sprites().empty()) {
				LOG_ERROR("Selected sprite sheet has no sprites");
				return std::nullopt;
			}

			// Load image
			const auto& resourcePath = spriteSheet.resource();
			const sdl::SurfaceUniquePtr surface(IMG_Load(resourcePath.c_str()));
			if (!surface) {
				LOG_ERROR("Unable to load image", resourcePath, IMG_GetError());
				return std::nullopt;
			}
			_texture = sdl::TextureUniquePtr{SDL_CreateTextureFromSurface(M2_GAME.renderer, surface.get())};
			if (!_texture) {
				LOG_ERROR("Unable to create texture from surface", SDL_GetError());
				return std::nullopt;
			}
			_textureDimensions = {surface->w, surface->h};
			_ppm = spriteSheet.ppm();
			_selected_resource = resource;

			// Enable selection
			M2_LEVEL.EnablePrimarySelection(M2_GAME.Dimensions().Game());

			return spriteSheet;
		}
	}
	return std::nullopt;
}
void bulk_sheet_editor::State::LookUpAndStoreSpriteRect(std::optional<m2g::pb::SpriteType> sprite) {
	if (sprite) {
		for (const auto& spriteSheets = this->ReadSpriteSheetsFromFile(); const auto& spriteSheet : spriteSheets.sheets()) {
			if (spriteSheet.resource() == _selected_resource) {
				for (const auto& spriteObj : spriteSheet.sprites()) {
					if (spriteObj.has_regular() && spriteObj.type() == *sprite) {
						_savedSpriteRect = RectI{spriteObj.regular().rect()};
						return;
					}
				}
			}
		}
	}
	_savedSpriteRect = std::nullopt;
}
void bulk_sheet_editor::State::SetRect(const m2g::pb::SpriteType sprite, const RectI& rect) {
	sheet_editor::modify_sprite_in_sheet(_sprite_sheets_path, sprite, [&](pb::Sprite& sprite_) {
		sprite_.mutable_regular()->mutable_rect()->set_x(rect.x);
		sprite_.mutable_regular()->mutable_rect()->set_y(rect.y);
		sprite_.mutable_regular()->mutable_rect()->set_w(rect.w);
		sprite_.mutable_regular()->mutable_rect()->set_h(rect.h);
	});
	IF(M2_LEVEL.PrimarySelection())->Reset();
}
void bulk_sheet_editor::State::Reset(const m2g::pb::SpriteType sprite) {
	sheet_editor::modify_sprite_in_sheet(_sprite_sheets_path, sprite, [&](pb::Sprite& sprite_) {
		sprite_.mutable_regular()->clear_rect();
		sprite_.mutable_regular()->clear_center_to_origin_vec_px();
	});
	IF(M2_LEVEL.PrimarySelection())->Reset();
}

void bulk_sheet_editor::State::Draw() const {
	// Draw texture
	const auto offset = VecF{-0.5f, -0.5f};
	const auto textureTopLeftOutputPosition = ScreenOriginToPositionVecPx(offset);
	const auto textureBottomRightOutputPosition = ScreenOriginToPositionVecPx(static_cast<VecF>(_textureDimensions) + offset);
	const SDL_Rect dstRect = {
			RoundI(textureTopLeftOutputPosition.x), RoundI(textureTopLeftOutputPosition.y),
			RoundI(textureBottomRightOutputPosition.x - textureTopLeftOutputPosition.x),
			RoundI(textureBottomRightOutputPosition.y - textureTopLeftOutputPosition.y)};
	SDL_RenderCopy(M2_GAME.renderer, _texture.get(), nullptr, &dstRect);
	// Draw currectly selected sprite's rect
	if (_savedSpriteRect) {
		auto world_coordinates_m = RectF{
			F(_savedSpriteRect->x),
			F(_savedSpriteRect->y),
			F(_savedSpriteRect->w),
			F(_savedSpriteRect->h)};
		Graphic::ColorRect(world_coordinates_m.Shift({-0.5f, -0.5f}), CONFIRMED_SELECTION_COLOR);
	}
	// Draw selection
	if (const auto* selection = M2_LEVEL.PrimarySelection(); selection) {
		if (const auto cellSelection = selection->CellSelectionRectM()) {
			Graphic::ColorRect(*cellSelection, SELECTION_COLOR);
		}
	}
	// Draw pixel grid lines
	Graphic::DrawGridLines(-0.5f, 1.0f, {127, 127, 255, 80});
	// Draw PPM grid lines
	Graphic::DrawGridLines(-0.5f, F(_ppm));
	// Draw sheet boundaries
	Graphic::DrawVerticalLine(-0.5f, {255, 0, 0, 255});
	Graphic::DrawHorizontalLine(-0.5f, {255, 0, 0, 255});
	Graphic::DrawVerticalLine(F(_textureDimensions.x) - 0.5f, {255, 0, 0, 255});
	Graphic::DrawHorizontalLine(F(_textureDimensions.y) - 0.5f, {255, 0, 0, 255});
}
