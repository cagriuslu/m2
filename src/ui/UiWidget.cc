#include <m2/Game.h>
#include <m2/ui/UiWidget.h>
#include <m2/ui/UiPanel.h>
#include <m2/thirdparty/video/TextRendering.h>
#include <m2/thirdparty/video/Shapes.h>

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

m2::RectI m2::UiWidget::calculate_wrapped_text_rect(VecI textTextureDimensions, RectI drawable_area, TextHorizontalAlignment align_h, TextVerticalAlignment align_v) {
	if (drawable_area.w < textTextureDimensions.x) {
		throw M2_ERROR("Font should have been generated at most as wide as the drawable area");
	}
	RectI unaligned = RectI{-1, -1, textTextureDimensions.x, textTextureDimensions.y};

	RectI horizontally_aligned;
	switch (align_h) {
		case TextHorizontalAlignment::LEFT:
			horizontally_aligned = unaligned.AlignLeftTo(drawable_area.x);
			break;
		case TextHorizontalAlignment::RIGHT:
			horizontally_aligned = unaligned.AlignRightTo(drawable_area.GetX2());
			break;
		default:
			horizontally_aligned = unaligned.AlignLeftTo(drawable_area.GetXCenter() - textTextureDimensions.x / 2);
			break;
	}

	switch (align_v) {
		case TextVerticalAlignment::TOP:
			return horizontally_aligned.AlignTopTo(drawable_area.y);
		case TextVerticalAlignment::BOTTOM:
			return horizontally_aligned.AlignBottomTo(drawable_area.GetY2());
		default:
			return horizontally_aligned.AlignTopTo(drawable_area.GetYCenter() - textTextureDimensions.y / 2);
	}
}

m2::RectI m2::UiWidget::calculate_filled_text_rect(const RectI drawable_area, const TextHorizontalAlignment align, const char* text) {
	// Fit the font into the drawable_area with correct aspect ratio
	const auto defaultFontRenderedSize = thirdparty::video::CalculateRenderedUtf8Size(M2_GAME.font, M2G_PROXY.default_font_size, text);
	const auto unaligned_destination = drawable_area.TrimToAspectRatio(defaultFontRenderedSize.x, defaultFontRenderedSize.y);

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

void UiWidget::draw_rectangle(const RectI& rect, const RGBA& color) {
	thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), rect, color);
}

void UiWidget::DrawSpriteOrTextLabel(const std::variant<Sprite, pb::TextLabel>& spriteOrTextLabel, const RectI& dst_rect) {
	RectI src_rect;
	const thirdparty::video::Texture* texture;
	if (std::holds_alternative<Sprite>(spriteOrTextLabel)) {
		src_rect = std::get<Sprite>(spriteOrTextLabel).GetRect();
		texture = &std::get<Sprite>(spriteOrTextLabel).GetTexture();
	} else {
		src_rect = M2_GAME.GetTextLabelCache().Create(std::get<pb::TextLabel>(spriteOrTextLabel).text(), M2G_PROXY.default_font_size);
		texture = &M2_GAME.GetTextLabelCache().Texture();
	}

	const auto sprite_aspect_ratio = ToFloat(src_rect.w) / ToFloat(src_rect.h);
	const auto widget_aspect_ratio = ToFloat(dst_rect.w) / ToFloat(dst_rect.h);
	const float sprite_size_multiplier =
		sprite_aspect_ratio < widget_aspect_ratio // Compare aspect ratios of sprite and widget
		? ToFloat(dst_rect.h) / ToFloat(src_rect.h) // UiWidget is wider than the sprite
		: ToFloat(dst_rect.w) / ToFloat(src_rect.w); // Sprite is wider than the widget

	const auto actual_dst_rect = RectI{
		dst_rect.x + (dst_rect.w - RoundI(src_rect.w * sprite_size_multiplier)) / 2,
		dst_rect.y + (dst_rect.h - RoundI(src_rect.h * sprite_size_multiplier)) / 2,
		RoundI(src_rect.w * sprite_size_multiplier),
		RoundI(src_rect.h * sprite_size_multiplier)};

	texture->Render(M2_GAME.GetRenderer(), src_rect, actual_dst_rect);
}

void UiWidget::draw_border(const RectI& rect, int vertical_border_width_px, int horizontal_border_width_px, const RGBA& color) {
	thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), RectI{rect.x, rect.y, vertical_border_width_px, rect.h}, color);
	thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), RectI{rect.GetX2() - vertical_border_width_px, rect.y, vertical_border_width_px, rect.h}, color);
	thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), RectI{rect.x, rect.y, rect.w, horizontal_border_width_px}, color);
	thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), RectI{rect.x, rect.GetY2() - horizontal_border_width_px, rect.w, horizontal_border_width_px}, color);
}
