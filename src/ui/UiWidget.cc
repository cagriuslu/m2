#include <m2/Game.h>
#include <m2/ui/UiWidget.h>
#include <m2/ui/UiPanel.h>

using namespace m2;

UiWidget::UiWidget(UiPanel* parent, const UiWidgetBlueprint* blueprint) : _parent(parent), enabled(blueprint->initially_enabled), blueprint(blueprint) {}
UiWidget::~UiWidget() = default;

void UiWidget::SetRect(const RectI& rect_px) {
	const auto oldRect = _rect_px;
	_rect_px = rect_px;
	OnResize(oldRect, _rect_px);
}
UiAction UiWidget::HandleEvents(Events& e) {
	if (const auto mouseIsAbove = Rect().DoesContain(e.MousePosition()); mouseIsAbove && not _hoverActive) {
		OnHover();
		_hoverActive = true;
	} else if (not mouseIsAbove && _hoverActive) {
		_hoverActive = false;
		OffHover();
	}
	return OnEvent(e);
}
void UiWidget::SetFocusState(bool newState) {
	_focused = newState;
	OnFocusChange();
}
UiAction UiWidget::UpdateContents() {
	return OnUpdate();
}
void UiWidget::Draw() {
	OnDraw();
}

void m2::UiWidget::draw_background_color() const {
	const auto& color = blueprint->background_color;
	if (color.r || color.g || color.b || color.a) {
		draw_rectangle(Rect(), color);
	}
}

int m2::UiWidget::vertical_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0;
	} else {
		return std::max(1, RoundI(vertical_pixels_per_unit() * blueprint->border_width));
	}
}

int m2::UiWidget::horizontal_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0;
	} else {
		return std::max(1, RoundI(horizontal_pixels_per_unit() * blueprint->border_width));
	}
}

int m2::UiWidget::vertical_padding_width_px() const {
	if (blueprint->padding_width == 0.0f) {
		return 0;
	} else {
		return RoundI(vertical_pixels_per_unit() * blueprint->padding_width);
	}
}

int m2::UiWidget::horizontal_padding_width_px() const {
	if (blueprint->padding_width == 0.0f) {
		return 0;
	} else {
		return RoundI(horizontal_pixels_per_unit() * blueprint->padding_width);
	}
}

m2::RectI m2::UiWidget::drawable_area() const {
	auto vertical_excess = vertical_border_width_px() + vertical_padding_width_px();
	auto horizontal_excess = horizontal_border_width_px() + horizontal_padding_width_px();
	return Rect().TrimLeft(vertical_excess).TrimRight(vertical_excess).TrimTop(horizontal_excess).TrimBottom(horizontal_excess);
}

m2::RectI m2::UiWidget::calculate_wrapped_text_rect(SDL_Texture* text_texture, RectI drawable_area, TextHorizontalAlignment align_h, TextVerticalAlignment align_v) {
	auto text_texture_dimensions = sdl::texture_dimensions(text_texture);
	if (drawable_area.w < text_texture_dimensions.x) {
		throw M2_ERROR("Font should have been generated at most as wide as the drawable area");
	}
	RectI unaligned = RectI{-1, -1, text_texture_dimensions.x, text_texture_dimensions.y};

	RectI horizontally_aligned;
	switch (align_h) {
		case TextHorizontalAlignment::LEFT:
			horizontally_aligned = unaligned.AlignLeftTo(drawable_area.x);
			break;
		case TextHorizontalAlignment::RIGHT:
			horizontally_aligned = unaligned.AlignRightTo(drawable_area.GetX2());
			break;
		default:
			horizontally_aligned = unaligned.AlignLeftTo(drawable_area.GetXCenter() - text_texture_dimensions.x / 2);
			break;
	}

	switch (align_v) {
		case TextVerticalAlignment::TOP:
			return horizontally_aligned.AlignTopTo(drawable_area.y);
		case TextVerticalAlignment::BOTTOM:
			return horizontally_aligned.AlignBottomTo(drawable_area.GetY2());
		default:
			return horizontally_aligned.AlignTopTo(drawable_area.GetYCenter() - text_texture_dimensions.y / 2);
	}
}

m2::RectI m2::UiWidget::calculate_filled_text_rect(RectI drawable_area, TextHorizontalAlignment align, int text_length) {
	// Fit the font into the drawable_area with correct aspect ratio
	auto unaligned_destination = drawable_area.TrimToAspectRatio(
		I(text_length * M2_GAME.FontLetterWidthToHeightRatio().GetN()),
			I(M2_GAME.FontLetterWidthToHeightRatio().GetD()));

	// If drawable area is wider than the font, horizontal alignment is necessary.
	switch (align) {
		case TextHorizontalAlignment::LEFT:
			return unaligned_destination.AlignLeftTo(drawable_area.x);
		case TextHorizontalAlignment::RIGHT:
			return unaligned_destination.AlignRightTo(drawable_area.GetX2());
		default:
			return unaligned_destination;
	}
}

void UiWidget::draw_rectangle(const RectI& rect, const SDL_Color& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);

	const auto sdl_rect = static_cast<SDL_Rect>(rect);
	SDL_RenderFillRect(M2_GAME.renderer, &sdl_rect);
}

void UiWidget::DrawSpriteOrTextLabel(const std::variant<Sprite, pb::TextLabel>& spriteOrTextLabel, const RectI& dst_rect) {
	SDL_Rect src_rect;
	SDL_Texture* texture;
	if (std::holds_alternative<Sprite>(spriteOrTextLabel)) {
		src_rect = static_cast<SDL_Rect>(std::get<Sprite>(spriteOrTextLabel).GetRect());
		texture = std::get<Sprite>(spriteOrTextLabel).GetTexture();
	} else {
		src_rect = static_cast<SDL_Rect>(M2_GAME.GetTextLabelCache().Create(std::get<pb::TextLabel>(spriteOrTextLabel).text(), M2G_PROXY.default_font_size));
		texture = M2_GAME.GetTextLabelCache().Texture();
	}

	const auto sprite_aspect_ratio = F(src_rect.w) / F(src_rect.h);
	const auto widget_aspect_ratio = F(dst_rect.w) / F(dst_rect.h);
	const float sprite_size_multiplier =
		sprite_aspect_ratio < widget_aspect_ratio  // Compare aspect ratios of sprite and widget
		? F(dst_rect.h) / F(src_rect.h) // UiWidget is wider than the sprite
		: F(dst_rect.w) / F(src_rect.w);  // Sprite is wider than the widget

	const auto actual_dst_rect = SDL_Rect{
		.x = dst_rect.x + (dst_rect.w - RoundI(src_rect.w * sprite_size_multiplier)) / 2,
		.y = dst_rect.y + (dst_rect.h - RoundI(src_rect.h * sprite_size_multiplier)) / 2,
		.w = RoundI(src_rect.w * sprite_size_multiplier),
		.h = RoundI(src_rect.h * sprite_size_multiplier)};

	SDL_RenderCopy(M2_GAME.renderer, texture, &src_rect, &actual_dst_rect);
}

void UiWidget::draw_border(const RectI& rect, int vertical_border_width_px, int horizontal_border_width_px, const SDL_Color& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);

	SDL_Rect left_right_top_bottom[4] = {
		{.x = rect.x, .y = rect.y, .w = vertical_border_width_px, .h = rect.h},
		{.x = rect.GetX2() - vertical_border_width_px, .y = rect.y, .w = vertical_border_width_px, .h = rect.h},
		{.x = rect.x, .y = rect.y, .w = rect.w, .h = horizontal_border_width_px},
		{.x = rect.x, .y = rect.GetY2() - horizontal_border_width_px, .w = rect.w, .h = horizontal_border_width_px}
	};
	SDL_RenderFillRects(M2_GAME.renderer, left_right_top_bottom, 4);
}
