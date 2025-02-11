#include <m2/sheet_editor/State.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/game/Selection.h>
#include <m2/game/object/Line.h>
#include <m2/protobuf/Detail.h>
#include <SDL2/SDL_image.h>

using namespace m2;
using namespace m2::sheet_editor;

namespace {
	constexpr SDL_Color SELECTION_COLOR = {0, 127, 255, 180};
	constexpr SDL_Color YELLOW_SELECTION_COLOR = {127, 127, 0, 180};
	constexpr SDL_Color RED_SELECTION_COLOR = {255, 0, 0, 180};
	constexpr SDL_Color CONFIRMED_SELECTION_COLOR = {0, 255, 0, 80};
	constexpr SDL_Color CROSS_COLOR = {0, 127, 255, 255};
	constexpr SDL_Color CONFIRMED_CROSS_COLOR = {0, 255, 0, 255};
}  // namespace

State::ForegroundCompanionMode::ForegroundCompanionMode() {
	const auto& sprite = std::get<State>(M2_LEVEL.stateVariant).selected_sprite();
	if (sprite.has_regular() && sprite.regular().foreground_companion_rects_size()) {
		// Iterate over rects
		for (const auto& rect : sprite.regular().foreground_companion_rects()) {
			current_rects.emplace_back(rect);
		}
		// Set center
		current_center = VecF{sprite.regular().foreground_companion_center_to_origin_vec_px()};
	}
	M2_LEVEL.EnablePrimarySelection(M2_GAME.Dimensions().Game());
}
State::ForegroundCompanionMode::~ForegroundCompanionMode() { M2_LEVEL.DisablePrimarySelection(); }
void State::ForegroundCompanionMode::on_draw() const {
	// Draw selection
	if (const auto cellSelection = M2_LEVEL.PrimarySelection()->CellSelectionRectM()) {
		Graphic::color_rect(*cellSelection, SELECTION_COLOR);
	}
	// Draw center selection
	if (secondary_selection_position) {
		Graphic::draw_cross(*secondary_selection_position, CROSS_COLOR);
	}

	for (const auto& rect : current_rects) {
		Graphic::color_rect(RectF{rect}.shift({-0.5f, -0.5f}), CONFIRMED_SELECTION_COLOR);
	}
	if (current_center) {
		auto sprite_center = std::get<State>(M2_LEVEL.stateVariant).selected_sprite_center();
		Graphic::draw_cross(sprite_center + *current_center, CONFIRMED_CROSS_COLOR);
	}
}
void State::ForegroundCompanionMode::add_rect() {
	if (auto* selection = M2_LEVEL.PrimarySelection(); selection->IsComplete()) {
		const auto intSelection = selection->IntegerSelectionRectM();
		std::get<State>(M2_LEVEL.stateVariant).modify_selected_sprite([&](pb::Sprite& sprite) {
			auto* new_rect = sprite.mutable_regular()->add_foreground_companion_rects();
			new_rect->set_x(intSelection->x);
			new_rect->set_y(intSelection->y);
			new_rect->set_w(intSelection->w);
			new_rect->set_h(intSelection->h);
		});
		current_rects.emplace_back(*intSelection);
		selection->Reset();
	}
}
void State::ForegroundCompanionMode::set_center() {
	// Store center selection
	if (secondary_selection_position) {
		auto center_offset = *secondary_selection_position -
		    std::get<State>(M2_LEVEL.stateVariant).selected_sprite_center();  // new offset from sprite center
		std::get<State>(M2_LEVEL.stateVariant).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_foreground_companion_center_to_origin_vec_px()->set_x(center_offset.x);
			sprite.mutable_regular()->mutable_foreground_companion_center_to_origin_vec_px()->set_y(center_offset.y);
		});
		current_center = center_offset;
		secondary_selection_position = std::nullopt;
	}
}
void State::ForegroundCompanionMode::reset() {
	std::get<State>(M2_LEVEL.stateVariant).modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_foreground_companion_center_to_origin_vec_px();
		sprite.mutable_regular()->clear_foreground_companion_rects();
	});
	current_rects.clear();
	current_center = std::nullopt;
	M2_LEVEL.PrimarySelection()->Reset();
	secondary_selection_position = std::nullopt;
}

State::BackgroundColliderMode::BackgroundColliderMode() {
	const auto& sprite = std::get<State>(M2_LEVEL.stateVariant).selected_sprite();
	if (sprite.regular().has_background_collider()) {
		auto collider_origin =
		    VecF{sprite.regular().center_to_origin_vec_px()} + VecF{sprite.regular().background_collider().sprite_origin_to_collider_origin_vec_px()};
		if (sprite.regular().background_collider().has_rect_dims_px()) {
			current_rect = RectF::centered_around(collider_origin, VecF{sprite.regular().background_collider().rect_dims_px()});
		} else if (sprite.regular().background_collider().has_circ_radius_px()) {
			auto radius = sprite.regular().background_collider().circ_radius_px();
			current_circ = CircF{collider_origin, radius};
		}
	}
	M2_LEVEL.EnablePrimarySelection(M2_GAME.Dimensions().Game());
	M2_LEVEL.EnableSecondarySelection(M2_GAME.Dimensions().Game());
}
State::BackgroundColliderMode::~BackgroundColliderMode() {
	M2_LEVEL.DisablePrimarySelection();
	M2_LEVEL.DisableSecondarySelection();
}
void State::BackgroundColliderMode::on_draw() const {
	// Draw rect selection
	if (const auto halfCellSelection = M2_LEVEL.PrimarySelection()->HalfCellSelectionRectM()) {
		Graphic::color_rect(*halfCellSelection, YELLOW_SELECTION_COLOR);
	}
	// Draw circ selection
	if (const auto halfCellSelection = M2_LEVEL.SecondarySelection()->HalfCellSelectionRectM()) {
		Graphic::color_disk(halfCellSelection->top_left(), halfCellSelection->DiagonalLength(), RED_SELECTION_COLOR);
	}

	if (current_rect) {
		auto sprite_center = std::get<State>(M2_LEVEL.stateVariant).selected_sprite_center();
		auto rect = current_rect->shift(sprite_center);
		Graphic::color_rect(rect, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(rect.center(), CONFIRMED_CROSS_COLOR);
	}
	if (current_circ) {
		// Find location of the circle
		auto sprite_center = std::get<State>(M2_LEVEL.stateVariant).selected_sprite_center();
		auto circ = CircF{current_circ->center + sprite_center, current_circ->r};
		Graphic::color_disk(circ.center, circ.r, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(circ.center, CONFIRMED_CROSS_COLOR);
	}
}
void State::BackgroundColliderMode::set() {
	if (M2_LEVEL.PrimarySelection()->IsComplete()) {
		const auto halfCellSelection = M2_LEVEL.PrimarySelection()->HalfCellSelectionRectM();
		auto offset = halfCellSelection->center() -
		    std::get<State>(M2_LEVEL.stateVariant).selected_sprite_origin();  // new offset from sprite origin
		auto dims = VecF{halfCellSelection->w, halfCellSelection->h};  // new dims
		std::get<State>(M2_LEVEL.stateVariant).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_background_collider()->mutable_sprite_origin_to_collider_origin_vec_px()->set_x(offset.x);
			sprite.mutable_regular()->mutable_background_collider()->mutable_sprite_origin_to_collider_origin_vec_px()->set_y(offset.y);
			sprite.mutable_regular()->mutable_background_collider()->mutable_rect_dims_px()->set_w(dims.x);
			sprite.mutable_regular()->mutable_background_collider()->mutable_rect_dims_px()->set_h(dims.y);
		});
		current_rect = halfCellSelection->shift_origin(std::get<State>(M2_LEVEL.stateVariant).selected_sprite_center());
		current_circ = std::nullopt;
		M2_LEVEL.PrimarySelection()->Reset();
	} else if (M2_LEVEL.SecondarySelection()->IsComplete()) {
		const auto halfCellSelection = M2_LEVEL.SecondarySelection()->HalfCellSelectionRectM();
		auto center = halfCellSelection->top_left();
		auto radius = halfCellSelection->DiagonalLength();
		auto offset = center -
		    std::get<State>(M2_LEVEL.stateVariant).selected_sprite_origin();  // new offset from sprite origin
		std::get<State>(M2_LEVEL.stateVariant).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_background_collider()->mutable_sprite_origin_to_collider_origin_vec_px()->set_x(offset.x);
			sprite.mutable_regular()->mutable_background_collider()->mutable_sprite_origin_to_collider_origin_vec_px()->set_y(offset.y);
			sprite.mutable_regular()->mutable_background_collider()->set_circ_radius_px(radius);
		});
		current_rect = std::nullopt;
		current_circ = CircF{center - std::get<State>(M2_LEVEL.stateVariant).selected_sprite_center(), radius};
		M2_LEVEL.SecondarySelection()->Reset();
	}
}
void State::BackgroundColliderMode::reset() {
	std::get<sheet_editor::State>(M2_LEVEL.stateVariant).modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_background_collider();
	});
	current_rect = std::nullopt;
	current_circ = std::nullopt;
	M2_LEVEL.PrimarySelection()->Reset();
	M2_LEVEL.SecondarySelection()->Reset();
}

State::ForegroundColliderMode::ForegroundColliderMode() {
	const auto& sprite = std::get<State>(M2_LEVEL.stateVariant).selected_sprite();
	if (sprite.regular().has_foreground_collider()) {
		auto collider_origin =
		    VecF{sprite.regular().center_to_origin_vec_px()} + VecF{sprite.regular().foreground_collider().sprite_origin_to_collider_origin_vec_px()};
		if (sprite.regular().foreground_collider().has_rect_dims_px()) {
			current_rect = RectF::centered_around(collider_origin, VecF{sprite.regular().foreground_collider().rect_dims_px()});
		} else if (sprite.regular().foreground_collider().has_circ_radius_px()) {
			auto radius = sprite.regular().foreground_collider().circ_radius_px();
			current_circ = CircF{collider_origin, radius};
		}
	}
	// Enable selection
	M2_LEVEL.EnablePrimarySelection(M2_GAME.Dimensions().Game());
	M2_LEVEL.EnableSecondarySelection(M2_GAME.Dimensions().Game());
}
State::ForegroundColliderMode::~ForegroundColliderMode() {
	M2_LEVEL.DisablePrimarySelection();
	M2_LEVEL.DisableSecondarySelection();
}
void State::ForegroundColliderMode::on_draw() const {
	// Draw rect selection
	if (const auto halfCellSelection = M2_LEVEL.PrimarySelection()->HalfCellSelectionRectM()) {
		Graphic::color_rect(*halfCellSelection, YELLOW_SELECTION_COLOR);
	}
	// Draw circ selection
	if (const auto halfCellSelection = M2_LEVEL.SecondarySelection()->HalfCellSelectionRectM()) {
		Graphic::color_disk(halfCellSelection->top_left(), halfCellSelection->DiagonalLength(), RED_SELECTION_COLOR);
	}

	if (current_rect) {
		auto sprite_center = std::get<State>(M2_LEVEL.stateVariant).selected_sprite_center();
		auto rect = current_rect->shift(sprite_center);
		Graphic::color_rect(rect, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(rect.center(), CONFIRMED_CROSS_COLOR);
	}
	if (current_circ) {
		auto sprite_center = std::get<State>(M2_LEVEL.stateVariant).selected_sprite_center();
		auto circ = CircF{current_circ->center + sprite_center, current_circ->r};
		Graphic::color_disk(circ.center, circ.r, CONFIRMED_SELECTION_COLOR);
		Graphic::draw_cross(circ.center, CONFIRMED_CROSS_COLOR);
	}
}
void State::ForegroundColliderMode::set() {
	if (M2_LEVEL.PrimarySelection()->IsComplete()) {
		const auto halfCellSelection = M2_LEVEL.PrimarySelection()->HalfCellSelectionRectM();
		auto offset = halfCellSelection->center() -
		    std::get<State>(M2_LEVEL.stateVariant).selected_sprite_origin();  // new offset from sprite origin
		auto dims = VecF{halfCellSelection->w, halfCellSelection->h};  // new dims
		std::get<State>(M2_LEVEL.stateVariant).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_sprite_origin_to_collider_origin_vec_px()->set_x(offset.x);
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_sprite_origin_to_collider_origin_vec_px()->set_y(offset.y);
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_rect_dims_px()->set_w(dims.x);
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_rect_dims_px()->set_h(dims.y);
		});
		current_rect = halfCellSelection->shift_origin(std::get<State>(M2_LEVEL.stateVariant).selected_sprite_center());
		current_circ = std::nullopt;
		M2_LEVEL.PrimarySelection()->Reset();
	} else if (M2_LEVEL.SecondarySelection()->IsComplete()) {
		const auto halfCellSelection = M2_LEVEL.SecondarySelection()->HalfCellSelectionRectM();
		auto center = halfCellSelection->top_left();
		auto radius = halfCellSelection->DiagonalLength();
		auto offset = center -
		    std::get<State>(M2_LEVEL.stateVariant).selected_sprite_origin();  // new offset from sprite origin
		std::get<State>(M2_LEVEL.stateVariant).modify_selected_sprite([&](pb::Sprite& sprite) {
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_sprite_origin_to_collider_origin_vec_px()->set_x(offset.x);
			sprite.mutable_regular()->mutable_foreground_collider()->mutable_sprite_origin_to_collider_origin_vec_px()->set_y(offset.y);
			sprite.mutable_regular()->mutable_foreground_collider()->set_circ_radius_px(radius);
		});
		current_rect = std::nullopt;
		current_circ = CircF{center - std::get<State>(M2_LEVEL.stateVariant).selected_sprite_center(), radius};
		M2_LEVEL.SecondarySelection()->Reset();
	}
}
void State::ForegroundColliderMode::reset() {
	std::get<sheet_editor::State>(M2_LEVEL.stateVariant).modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_foreground_collider();
	});
	current_rect = std::nullopt;
	current_circ = std::nullopt;
	M2_LEVEL.PrimarySelection()->Reset();
	M2_LEVEL.SecondarySelection()->Reset();
}

expected<m2::sheet_editor::State> m2::sheet_editor::State::create(const std::filesystem::path& path) {
	// If path exists,
	if (std::filesystem::exists(path)) {
		// Check if the file is a valid pb::SpriteSheets
		if (auto msg = pb::json_file_to_message<pb::SpriteSheets>(path); !msg) {
			return make_unexpected(msg.error());
		}
	}
	if (auto persistentSpriteSheets = pb::PersistentObject<pb::SpriteSheets>::LoadFile(path)) {
		return State{std::move(*persistentSpriteSheets)};
	} else {
		return make_unexpected(persistentSpriteSheets.error());
	}
}

const pb::Sprite& m2::sheet_editor::State::selected_sprite() const {
	const auto& sheets = SpriteSheets();
	for (const auto& sheet : sheets.sheets()) {
		for (const auto& sprite : sheet.sprites()) {
			if (sprite.type() == _selected_sprite_type) {
				return sprite;
			}
		}
	}
	throw M2_ERROR("Sprite sheet does not contain selected sprite");
}

void m2::sheet_editor::State::modify_selected_sprite(const std::function<void(pb::Sprite&)>& modifier) {
	ModifySpriteInSheets(_persistentSpriteSheets, _selected_sprite_type, modifier);
}

RectI m2::sheet_editor::State::selected_sprite_rect() const { return RectI{selected_sprite().regular().rect()}; }

VecF m2::sheet_editor::State::selected_sprite_center() const {
	// Rect needs to be shifted to fit into cells
	auto rect = RectF{selected_sprite_rect()}.shift({-0.5f, -0.5f});
	return rect.center();
}

VecF m2::sheet_editor::State::selected_sprite_origin() const {
	return selected_sprite_center() + VecF{selected_sprite().regular().center_to_origin_vec_px()};
}

void m2::sheet_editor::State::Select(m2g::pb::SpriteType spriteType) {
	const auto& spriteSheets = this->SpriteSheets();
	for (const auto& spriteSheet : spriteSheets.sheets()) {
		for (const auto& sprite : spriteSheet.sprites()) {
			if (sprite.type() == spriteType) {
				_selected_sprite_type = spriteType;
				// Load image
				const auto& resourcePath = spriteSheet.resource();
				sdl::SurfaceUniquePtr surface(IMG_Load(resourcePath.c_str()));
				if (!surface) {
					throw M2_ERROR("Unable to load image: " + resourcePath + ", " + IMG_GetError());
				}
				_texture = sdl::TextureUniquePtr{SDL_CreateTextureFromSurface(M2_GAME.renderer, surface.get())};
				if (!_texture) {
					throw M2_ERROR("Unable to create texture from surface: " + std::string{SDL_GetError()});
				}
				_textureDimensions = {surface->w, surface->h};
				_ppm = spriteSheet.ppm();

				// Move God to center if rect is already selected
				M2_PLAYER.position = selected_sprite_center();

				return;
			}
		}
	}
}

void m2::sheet_editor::State::deactivate_mode() { mode.emplace<std::monostate>(); }

void m2::sheet_editor::State::activate_foreground_companion_mode() { mode.emplace<ForegroundCompanionMode>(); }

void m2::sheet_editor::State::activate_background_collider_mode() { mode.emplace<BackgroundColliderMode>(); }

void m2::sheet_editor::State::activate_foreground_collider_mode() { mode.emplace<ForegroundColliderMode>(); }

void State::SetSpriteRect(const RectI& rect) {
	modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->mutable_rect()->set_x(rect.x);
		sprite.mutable_regular()->mutable_rect()->set_y(rect.y);
		sprite.mutable_regular()->mutable_rect()->set_w(rect.w);
		sprite.mutable_regular()->mutable_rect()->set_h(rect.h);
	});
}
void State::SetSpriteOrigin(const VecF& origin) {
	const auto rect = RectI{selected_sprite().regular().rect()};
	const auto rectCenter = RectF{rect}.shift({-0.5f, -0.5f}).center();
	const auto centerToOrigin = origin - rectCenter;
	modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->mutable_center_to_origin_vec_px()->set_x(centerToOrigin.x);
		sprite.mutable_regular()->mutable_center_to_origin_vec_px()->set_y(centerToOrigin.y);
	});
}
void State::ResetSpriteRectAndOrigin() {
	modify_selected_sprite([&](pb::Sprite& sprite) {
		sprite.mutable_regular()->clear_rect();
		sprite.mutable_regular()->clear_center_to_origin_vec_px();
	});
}
void State::AddForegroundCompanionRect(const RectI& rect) {}
void State::SetForegroundCompanionCenter(const VecF& center) {}
void State::RemoveForegroundCompanion() {}
void State::AddRectangleBackgroundCollider(const RectF& rect) {}
void State::AddCircleBackgroundCollider(const VecF& center, float radius) {}
void State::RemoveBackgroundColliders() {}
void State::AddRectangleForegroundCollider(const RectF& rect) {}
void State::AddCircleForegroundCollider(const VecF& center, float radius) {}
void State::RemoveForegroundColliders() {}

void State::Draw() const {
	// Draw texture
	const auto offset = VecF{-0.5f, -0.5f};
	const auto textureTopLeftOutputPosition = ScreenOriginToPositionVecPx(offset);
	const auto textureBottomRightOutputPosition = ScreenOriginToPositionVecPx(static_cast<VecF>(_textureDimensions) + offset);
	const SDL_Rect dstRect = {
		iround(textureTopLeftOutputPosition.x), iround(textureTopLeftOutputPosition.y),
		iround(textureBottomRightOutputPosition.x - textureTopLeftOutputPosition.x),
		iround(textureBottomRightOutputPosition.y - textureTopLeftOutputPosition.y)};
	SDL_RenderCopy(M2_GAME.renderer, _texture.get(), nullptr, &dstRect);

	if (M2_LEVEL.RightHud()->Name() == "RectModeRightHud") {
		// Draw rect selection
		if (const auto cellSelection = M2_LEVEL.PrimarySelection()->CellSelectionRectM()) {
			Graphic::color_rect(*cellSelection, SELECTION_COLOR);
		}
		// Draw origin selection
		if (const auto centerSelection = M2_LEVEL.SecondarySelection()->HalfCellSelectionRectM()) {
			Graphic::draw_cross(centerSelection->top_left(), CROSS_COLOR);
		}
		const auto& sprite = selected_sprite();
		const auto rect = RectI{sprite.regular().rect()};
		if (rect) {
			Graphic::color_rect(RectF{rect}.shift({-0.5f, -0.5f}), CONFIRMED_SELECTION_COLOR);
		}
		const auto rectCenter = RectF{rect}.shift({-0.5f, -0.5f}).center();
		const auto centerToOriginVecPx = VecF{sprite.regular().center_to_origin_vec_px()};
		Graphic::draw_cross(rectCenter + centerToOriginVecPx, CONFIRMED_CROSS_COLOR);
	} else {
		std::visit(overloaded{
			[](const auto& mode) { mode.on_draw(); },
			[](MAYBE const std::monostate&) {}},
			mode);
	}

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

void m2::sheet_editor::modify_sprite_in_sheet(
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
void sheet_editor::ModifySpriteInSheets(pb::PersistentObject<pb::SpriteSheets>& persistentObject,
		const m2g::pb::SpriteType spriteType, const std::function<void(pb::Sprite&)>& modifier) {
	auto expectSuccess = persistentObject.Mutate([&](pb::SpriteSheets& sheets) {
		for (auto& sheet : *sheets.mutable_sheets()) {
			for (auto& sprite : *sheet.mutable_sprites()) {
				if (sprite.type() == spriteType) {
					modifier(sprite);
					return;
				}
			}
		}
	});
	if (not expectSuccess) {
		throw M2_ERROR("Unable to mutate sprite sheets: " + expectSuccess.error());
	}
}
