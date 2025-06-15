#include <m2/sheet_editor/State.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/game/Selection.h>
#include <m2/protobuf/Detail.h>
#include <m2/ui/widget/TextSelection.h>
#include <SDL2/SDL_image.h>
#include <m2/sheet_editor/Ui.h>

using namespace m2;
using namespace m2::sheet_editor;

expected<State> State::create(const std::filesystem::path& path) {
	// If path exists,
	if (exists(path)) {
		// Check if the file is a valid pb::SpriteSheets
		if (auto msg = pb::json_file_to_message<pb::SpriteSheets>(path); !msg) {
			return make_unexpected(msg.error());
		}
	}
	if (auto persistentSpriteSheets = PersistentSpriteSheets::LoadFile(path)) {
		return State{std::move(*persistentSpriteSheets)};
	} else {
		return make_unexpected(persistentSpriteSheets.error());
	}
}

int State::SelectedSpriteFixtureCount() const {
	return SelectedSprite().regular().fixtures_size();
}
std::vector<pb::Fixture::FixtureTypeCase> State::SelectedSpriteFixtureTypes() const {
	return _persistentSpriteSheets.SpriteFixtureTypes(_selected_sprite_type);
}

void State::Select(const m2g::pb::SpriteType spriteType) {
	if (const auto* sheet = _persistentSpriteSheets.SpriteSheetPbWithSprite(spriteType)) {
		_selected_sprite_type = spriteType;
		// Load image
		const auto& resourcePath = sheet->resource();
		const sdl::SurfaceUniquePtr surface(IMG_Load(resourcePath.c_str()));
		if (!surface) {
			throw M2_ERROR("Unable to load image: " + resourcePath + ", " + IMG_GetError());
		}
		_texture = sdl::TextureUniquePtr{SDL_CreateTextureFromSurface(M2_GAME.renderer, surface.get())};
		if (!_texture) {
			throw M2_ERROR("Unable to create texture from surface: " + std::string{SDL_GetError()});
		}
		_textureDimensions = {surface->w, surface->h};
		_ppm = sheet->ppm();

		// Move God to center if rect is already selected
		M2_PLAYER.position = SelectedSpriteCenter();
	}
}

void State::SetSpriteRect(const RectI& rect) {
	ModifySelectedSprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->mutable_rect()->set_x(rect.x);
		sprite.mutable_regular()->mutable_rect()->set_y(rect.y);
		sprite.mutable_regular()->mutable_rect()->set_w(rect.w);
		sprite.mutable_regular()->mutable_rect()->set_h(rect.h);
	});
}
void State::SetSpriteOrigin(const VecF& origin) {
	const auto rect = RectI{SelectedSprite().regular().rect()};
	const auto rectCenter = RectF{rect}.Shift({-0.5f, -0.5f}).GetCenterPoint();
	const auto centerToOrigin = origin - rectCenter;
	ModifySelectedSprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->mutable_center_to_origin_vec_px()->set_x(centerToOrigin.x);
		sprite.mutable_regular()->mutable_center_to_origin_vec_px()->set_y(centerToOrigin.y);
	});
}
void State::ResetSpriteRectAndOrigin() {
	ModifySelectedSprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_rect();
		sprite.mutable_regular()->clear_center_to_origin_vec_px();
	});
}
void State::AddForegroundCompanionRect(const RectI& rect) {
	ModifySelectedSprite([&](pb::Sprite& sprite) {
		auto* newRect = sprite.mutable_regular()->mutable_foreground_companion()->add_rects();
		newRect->set_x(rect.x);
		newRect->set_y(rect.y);
		newRect->set_w(rect.w);
		newRect->set_h(rect.h);
	});
}
void State::SetForegroundCompanionOrigin(const VecF& origin) {
	const auto rect = RectI{SelectedSprite().regular().rect()};
	const auto rectCenter = RectF{rect}.Shift({-0.5f, -0.5f}).GetCenterPoint();
	const auto centerToOrigin = origin - rectCenter;
	ModifySelectedSprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->mutable_foreground_companion()->mutable_center_to_origin_vec_px()->set_x(centerToOrigin.x);
		sprite.mutable_regular()->mutable_foreground_companion()->mutable_center_to_origin_vec_px()->set_y(centerToOrigin.y);
	});
}
void State::ResetForegroundCompanion() {
	ModifySelectedSprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_foreground_companion();
	});
}
int State::AddFixture(const pb::Fixture::FixtureTypeCase type, const int insertIndex) {
	return _persistentSpriteSheets.AddFixtureToSprite(_selected_sprite_type, type, insertIndex);
}
void State::RemoveFixture(const int index) {
	_persistentSpriteSheets.RemoveFixtureFromSprite(_selected_sprite_type, index);
}
void State::StoreFixture(const int index, const RectF& rect, const VecF& point1, const VecF& point2) {
	ModifySelectedSprite([&](pb::Sprite& sprite) {
		if (auto* fixture = sprite.mutable_regular()->mutable_fixtures(index); fixture->has_rectangle()) {
			const auto spriteOriginToRectCenter = rect.GetCenterPoint() - SelectedSpriteOrigin();
			const auto dimensions = VecF{rect.w, rect.h};
			auto* rectangle = fixture->mutable_rectangle();
			rectangle->mutable_sprite_origin_to_fixture_center_vec_px()->set_x(spriteOriginToRectCenter.x);
			rectangle->mutable_sprite_origin_to_fixture_center_vec_px()->set_y(spriteOriginToRectCenter.y);
			rectangle->mutable_dims_px()->set_w(dimensions.x);
			rectangle->mutable_dims_px()->set_h(dimensions.y);
		} else if (fixture->has_circle()) {
			const auto& center = point1;
			const auto radius = (point2 - point1).GetLength();
			const auto spriteOriginToRectCenter = center - SelectedSpriteOrigin();
			auto* circle = fixture->mutable_circle();
			circle->mutable_sprite_origin_to_fixture_center_vec_px()->set_x(spriteOriginToRectCenter.x);
			circle->mutable_sprite_origin_to_fixture_center_vec_px()->set_y(spriteOriginToRectCenter.y);
			circle->set_radius_px(radius);
		} else if (fixture->has_chain()) {
			const auto spriteOriginToPoint = point1 - SelectedSpriteOrigin();
			auto* chain = fixture->mutable_chain();
			auto* point = chain->add_points();
			point->set_x(spriteOriginToPoint.x);
			point->set_y(spriteOriginToPoint.y);
		}
	});
}
void State::UndoChainFixturePoint(const int index) {
	ModifySelectedSprite([&](pb::Sprite& sprite) {
		if (auto* fixture = sprite.mutable_regular()->mutable_fixtures(index); fixture->has_chain()) {
			if (auto* chain = fixture->mutable_chain(); chain->points_size()) {
				auto* points = chain->mutable_points();
				points->erase(points->end() - 1);
			}
		}
	});
}

void State::Save() {
	if (const auto success = _persistentSpriteSheets.Save(); not success) {
		LOG_ERROR("Unable to save sprite sheet", success.error());
	}
}

void State::Draw() const {
	// Draw texture
	const auto offset = VecF{-0.5f, -0.5f};
	const auto textureTopLeftOutputPosition = ScreenOriginToPositionVecPx(offset);
	const auto textureBottomRightOutputPosition = ScreenOriginToPositionVecPx(static_cast<VecF>(_textureDimensions) + offset);
	const SDL_Rect dstRect = {RoundI(textureTopLeftOutputPosition.x), RoundI(textureTopLeftOutputPosition.y),
			RoundI(textureBottomRightOutputPosition.x - textureTopLeftOutputPosition.x),
			RoundI(textureBottomRightOutputPosition.y - textureTopLeftOutputPosition.y)};
	SDL_RenderCopy(M2_GAME.renderer, _texture.get(), nullptr, &dstRect);

	const auto& sprite = SelectedSprite();
	auto spriteOrigin = SelectedSpriteOrigin();

	if (M2_LEVEL.RightHud()->Name() == "RectModeRightHud") {
		// Draw selection
		if (const auto cellSelection = M2_LEVEL.PrimarySelection()->CellSelectionRectM()) {
			Graphic::ColorRect(*cellSelection, SELECTION_COLOR);
		}
		if (const auto centerSelection = M2_LEVEL.SecondarySelection()->HalfCellSelectionsM()) {
			Graphic::DrawCross(centerSelection->first, CROSS_COLOR);
		}
		// Draw already existing
		const auto rect = RectI{sprite.regular().rect()};
		if (rect) {
			Graphic::ColorRect(RectF{rect}.Shift({-0.5f, -0.5f}), CONFIRMED_SELECTION_COLOR);
		}
		const auto rectCenter = RectF{rect}.Shift({-0.5f, -0.5f}).GetCenterPoint();
		const auto centerToOriginVecPx = VecF{sprite.regular().center_to_origin_vec_px()};
		Graphic::DrawCross(rectCenter + centerToOriginVecPx, CONFIRMED_CROSS_COLOR);
	} else if (M2_LEVEL.RightHud()->Name() == "ForegroundCompanionModeRightHud") {
		// Draw selection
		if (const auto cellSelection = M2_LEVEL.PrimarySelection()->CellSelectionRectM()) {
			Graphic::ColorRect(*cellSelection, SELECTION_COLOR);
		}
		if (const auto centerSelection = M2_LEVEL.SecondarySelection()->HalfCellSelectionsM()) {
			Graphic::DrawCross(centerSelection->first, CROSS_COLOR);
		}
		// Draw already existing
		for (const auto& rect : sprite.regular().foreground_companion().rects()) {
			Graphic::ColorRect(RectF{rect}.Shift({-0.5f, -0.5f}), CONFIRMED_SELECTION_COLOR);
		}
		const auto rect = RectI{sprite.regular().rect()};
		const auto rectCenter = RectF{rect}.Shift({-0.5f, -0.5f}).GetCenterPoint();
		const auto fCompCenterToOriginVecPx = VecF{sprite.regular().foreground_companion().center_to_origin_vec_px()};
		Graphic::DrawCross(rectCenter + fCompCenterToOriginVecPx, CONFIRMED_CROSS_COLOR);
	} else if (M2_LEVEL.RightHud()->Name() == "FixtureModeRightHud") {
		if (const auto selectedFixtureIndex = M2_LEVEL.RightHud()->FindWidget<widget::TextSelection>("FixtureSelection")->GetSelectedIndexes();
				not selectedFixtureIndex.empty()) {
			const auto selectedIndex = selectedFixtureIndex[0];
			if (const auto& fixture = sprite.regular().fixtures(selectedIndex); fixture.has_rectangle()) {
				if (const auto halfCellSelection = M2_LEVEL.PrimarySelection()->HalfCellSelectionRectM()) {
					Graphic::ColorRect(*halfCellSelection, SELECTION_COLOR);
				}
			} else if (fixture.has_circle()) {
				if (const auto halfCellSelection = M2_LEVEL.PrimarySelection()->HalfCellSelectionsM()) {
					const auto center = halfCellSelection->first;
					const auto radius = center.GetDistanceTo(halfCellSelection->second);
					Graphic::ColorDisk(center, radius, SELECTION_COLOR);
				}
			} else if (fixture.has_chain()) {
				if (const auto halfCellSelection = M2_LEVEL.PrimarySelection()->HalfCellSelectionsM()) {
					const auto point = halfCellSelection->first;
					Graphic::DrawCross(point, CROSS_COLOR);
				}
			}
		}

		// Draw already existing fixtures
		for (const auto& fixture : sprite.regular().fixtures()) {
			if (fixture.has_rectangle()) {
				const auto& rect = fixture.rectangle();
				const auto rectF = RectF::CreateCenteredAround(spriteOrigin + VecF{rect.sprite_origin_to_fixture_center_vec_px()},
					rect.dims_px().w(), rect.dims_px().h());
				Graphic::ColorRect(rectF, CONFIRMED_SELECTION_COLOR);
			} else if (fixture.has_circle()) {
				const auto& circ = fixture.circle();
				const auto center = spriteOrigin + VecF{circ.sprite_origin_to_fixture_center_vec_px()};
				Graphic::ColorDisk(center, circ.radius_px(), CONFIRMED_SELECTION_COLOR);
			} else if (fixture.has_chain()) {
				if (const auto& points = fixture.chain().points(); points.size() == 1) {
					Graphic::DrawCross(spriteOrigin + VecF{points[0]}, CONFIRMED_CROSS_COLOR);
				} else if (1 < points.size()) {
					auto end = points.cend() - 1;
					for (auto it = points.cbegin(); it != end; ++it) {
						Graphic::DrawLine(spriteOrigin + VecF{*it}, spriteOrigin + VecF{*(it+1)}, CONFIRMED_CROSS_COLOR);
					}
				}
			}
		}
	}

	// Draw pixel grid lines
	Graphic::DrawGridLines(-0.5f, 1.0f, {255, 255, 255, 25});
	// Draw PPM grid lines
	Graphic::DrawGridLines(-0.5f, F(_ppm), {255, 255, 255, 100});
	// Draw sheet boundaries
	Graphic::DrawVerticalLine(-0.5f, {255, 0, 0, 255});
	Graphic::DrawHorizontalLine(-0.5f, {255, 0, 0, 255});
	Graphic::DrawVerticalLine(F(_textureDimensions.x) - 0.5f, {255, 0, 0, 255});
	Graphic::DrawHorizontalLine(F(_textureDimensions.y) - 0.5f, {255, 0, 0, 255});
}

void State::ModifySelectedSprite(const std::function<void(pb::Sprite&)>& modifier) {
	_persistentSpriteSheets.ModifySprite(_selected_sprite_type, modifier);
}

VecF State::SelectedSpriteCenter() const {
	// Rect needs to be shifted to fit into cells
	const auto rect = RectI{SelectedSprite().regular().rect()};
	const auto rectF = RectF{rect}.Shift({-0.5f, -0.5f});
	return rectF.GetCenterPoint();
}

VecF State::SelectedSpriteOrigin() const {
	return SelectedSpriteCenter() + VecF{SelectedSprite().regular().center_to_origin_vec_px()};
}

void sheet_editor::modify_sprite_in_sheet(
    const std::filesystem::path& path, const m2g::pb::SpriteType type, const std::function<void(pb::Sprite&)>& modifier) {
	// If path exists,
	if (exists(path)) {
		// Check if the file is a valid pb::SpriteSheets
		if (auto spriteSheets = pb::json_file_to_message<pb::SpriteSheets>(path); spriteSheets) {
			for (int i = 0; i < spriteSheets->sheets_size(); ++i) {
				auto* mutable_sheet = spriteSheets->mutable_sheets(i);
				for (int j = 0; j < mutable_sheet->sprites_size(); ++j) {
					if (auto* mutable_sprite = mutable_sheet->mutable_sprites(j); mutable_sprite->type() == type) {
						modifier(*mutable_sprite);
						message_to_json_file(*spriteSheets, path);
						return;
					}
				}
			}
			throw M2_ERROR("Sprite not found in SpriteSheets");
		}
		throw M2_ERROR("File is not a valid m2::pb::SpriteSheets: " + path.string());
	}
	throw M2_ERROR("Can't modify nonexistent file");
}
