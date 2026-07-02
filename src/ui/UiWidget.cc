#include <m2/Game.h>
#include <m2/ui/UiWidget.h>
#include <m2/ui/UiPanel.h>
#include <m2/thirdparty/video/TextRendering.h>
#include <m2/thirdparty/video/Shapes.h>

using namespace m2;

UiWidget::UiWidget(UiPanel* parent, const UiWidgetBlueprint* blueprint) : _parent(parent), enabled(blueprint->initially_enabled), blueprint(blueprint) {}
UiWidget::~UiWidget() = default;

void UiWidget::SetRect(const RectF& rectLpx) {
	const auto oldRectLpx = _rectLpx;
	_rectLpx = rectLpx;
	OnResize(oldRectLpx, _rectLpx);
}
UiAction UiWidget::HandleEvents(Events& e) {
	if (const auto mouseIsAbove = Rect().DoesContain(e.MousePositionLpx()); mouseIsAbove && not _hoverActive) {
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

float m2::UiWidget::vertical_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0.0f;
	} else {
		return std::max(1.0f, vertical_pixels_per_unit() * blueprint->border_width);
	}
}

float m2::UiWidget::horizontal_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0.0f;
	} else {
		return std::max(1.0f, horizontal_pixels_per_unit() * blueprint->border_width);
	}
}

float m2::UiWidget::vertical_padding_width_px() const {
	if (blueprint->padding_width == 0.0f) {
		return 0.0f;
	} else {
		return vertical_pixels_per_unit() * blueprint->padding_width;
	}
}

float m2::UiWidget::horizontal_padding_width_px() const {
	if (blueprint->padding_width == 0.0f) {
		return 0.0f;
	} else {
		return horizontal_pixels_per_unit() * blueprint->padding_width;
	}
}

m2::RectF m2::UiWidget::drawable_area() const {
	auto vertical_excess = vertical_border_width_px() + vertical_padding_width_px();
	auto horizontal_excess = horizontal_border_width_px() + horizontal_padding_width_px();
	return Rect().TrimLeft(vertical_excess).TrimRight(vertical_excess).TrimTop(horizontal_excess).TrimBottom(horizontal_excess);
}

m2::RectF m2::UiWidget::calculate_wrapped_text_rect(VecF textTextureDimensions, RectF drawableAreaLpx, TextHorizontalAlignment align_h, TextVerticalAlignment align_v) {
	if (drawableAreaLpx.w < textTextureDimensions.GetX()) {
		throw M2_ERROR("Font should have been generated at most as wide as the drawable area");
	}
	auto unaligned = RectF{-1.0f, -1.0f, textTextureDimensions.GetX(), textTextureDimensions.GetY()};

	RectF horizontally_aligned;
	switch (align_h) {
		case TextHorizontalAlignment::LEFT:
			horizontally_aligned = unaligned.AlignLeftTo(drawableAreaLpx.x);
			break;
		case TextHorizontalAlignment::RIGHT:
			horizontally_aligned = unaligned.AlignRightTo(drawableAreaLpx.GetX2());
			break;
		default:
			horizontally_aligned = unaligned.AlignLeftTo(drawableAreaLpx.GetXCenter() - textTextureDimensions.GetX() / 2.0f);
			break;
	}

	switch (align_v) {
		case TextVerticalAlignment::TOP:
			return horizontally_aligned.AlignTopTo(drawableAreaLpx.y);
		case TextVerticalAlignment::BOTTOM:
			return horizontally_aligned.AlignBottomTo(drawableAreaLpx.GetY2());
		default:
			return horizontally_aligned.AlignTopTo(drawableAreaLpx.GetYCenter() - textTextureDimensions.GetY() / 2.0f);
	}
}

m2::RectF m2::UiWidget::calculate_filled_text_rect(const RectF drawableAreaLpx, const TextHorizontalAlignment align, const char* text) {
	// Fit the font into the drawable_area with correct aspect ratio
	const auto defaultFontRenderedSize = thirdparty::video::CalculateRenderedUtf8Size(M2_GAME.font, M2G_PROXY.default_font_size, text);
	const auto unaligned_destination = drawableAreaLpx.TrimToAspectRatio(defaultFontRenderedSize.GetX(), defaultFontRenderedSize.GetY());

	// If drawable area is wider than the font, horizontal alignment is necessary.
	switch (align) {
		case TextHorizontalAlignment::LEFT:
			return unaligned_destination.AlignLeftTo(drawableAreaLpx.x);
		case TextHorizontalAlignment::RIGHT:
			return unaligned_destination.AlignRightTo(drawableAreaLpx.GetX2());
		default:
			return unaligned_destination;
	}
}

void UiWidget::draw_rectangle(const RectF& rectLpx, const RGBA& color) {
	thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), rectLpx, color);
}

void UiWidget::DrawSpriteOrTextLabel(const std::variant<Sprite, pb::TextLabel>& spriteOrTextLabel, const RectF& dstRectLpx) {
	RectI src_rect;
	const thirdparty::video::Texture* texture;
	if (std::holds_alternative<Sprite>(spriteOrTextLabel)) {
		src_rect = std::get<Sprite>(spriteOrTextLabel).GetRect();
		texture = &std::get<Sprite>(spriteOrTextLabel).GetTexture();
	} else {
		const auto dpiX = M2_GAME.GetRenderer().GetPixelsPerWindowUnit().GetX();
		src_rect = M2_GAME.GetTextLabelCache().Create(std::get<pb::TextLabel>(spriteOrTextLabel).text(), M2G_PROXY.default_font_size * dpiX);
		texture = &M2_GAME.GetTextLabelCache().Texture();
	}

	const auto sprite_aspect_ratio = ToFloat(src_rect.w) / ToFloat(src_rect.h);
	const auto widget_aspect_ratio = ToFloat(dstRectLpx.w) / ToFloat(dstRectLpx.h);
	const float sprite_size_multiplier =
		sprite_aspect_ratio < widget_aspect_ratio // Compare aspect ratios of sprite and widget
		? ToFloat(dstRectLpx.h) / ToFloat(src_rect.h) // UiWidget is wider than the sprite
		: ToFloat(dstRectLpx.w) / ToFloat(src_rect.w); // Sprite is wider than the widget

	const auto actual_dst_rect = RectF{
		dstRectLpx.x + (dstRectLpx.w - (static_cast<float>(src_rect.w) * sprite_size_multiplier)) / 2,
		dstRectLpx.y + (dstRectLpx.h - (static_cast<float>(src_rect.h) * sprite_size_multiplier)) / 2,
		static_cast<float>(src_rect.w) * sprite_size_multiplier,
		static_cast<float>(src_rect.h) * sprite_size_multiplier};

	texture->Render(M2_GAME.GetRenderer(), src_rect, actual_dst_rect);
}

void UiWidget::draw_border(const RectF& rectLpx, const float vertical_border_width_px, const float horizontal_border_width_px, const RGBA& color) {
	thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), RectF{rectLpx.x, rectLpx.y, vertical_border_width_px, rectLpx.h}, color);
	thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), RectF{rectLpx.GetX2() - vertical_border_width_px, rectLpx.y, vertical_border_width_px, rectLpx.h}, color);
	thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), RectF{rectLpx.x, rectLpx.y, rectLpx.w, horizontal_border_width_px}, color);
	thirdparty::video::FillRectangle(M2_GAME.GetRenderer(), RectF{rectLpx.x, rectLpx.GetY2() - horizontal_border_width_px, rectLpx.w, horizontal_border_width_px}, color);
}
