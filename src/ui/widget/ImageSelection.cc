#include <m2/Game.h>
#include <m2/ui/widget/Image.h>
#include <m2/ui/widget/ImageSelection.h>

using namespace m2;
using namespace m2::widget;

ImageSelection::ImageSelection(UiPanel* parent, const UiWidgetBlueprint* blueprint)
    : UiWidget(parent, blueprint),
      _plusTexture(m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, "+"))),
      _minusTexture(m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, "-"))) {
	select(0);
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

UiAction ImageSelection::OnEvent(Events& events) {
	auto buttons_rect = Rect().trim_top(Rect().w);
	auto inc_button_rect = buttons_rect.trim_left(buttons_rect.w / 2);
	auto dec_button_rect = buttons_rect.trim_right(buttons_rect.w / 2);

	const auto& image_selection = std::get<ImageSelectionBlueprint>(blueprint->variant);

	if (!_inc_depressed && events.PopMouseButtonPress(MouseButton::PRIMARY, inc_button_rect)) {
		_inc_depressed = true;
		_dec_depressed = false;
	} else if (!_dec_depressed && events.PopMouseButtonPress(MouseButton::PRIMARY, dec_button_rect)) {
		_dec_depressed = true;
		_inc_depressed = false;
	} else if (_inc_depressed && events.PopMouseButtonRelease(MouseButton::PRIMARY, inc_button_rect)) {
		_inc_depressed = false;
		if (_selection + 1 < image_selection.list.size()) {
			select(_selection + 1);
		}
	} else if (_dec_depressed && events.PopMouseButtonRelease(MouseButton::PRIMARY, dec_button_rect)) {
		_dec_depressed = false;
		if (0 < _selection) {
			select(_selection - 1);
		}
	} else {
		// Check if scrolled
		if (auto scroll_amount = events.PopMouseWheelVerticalScroll(Rect()); 0 < scroll_amount) {
			auto min_scroll_amount =
			    std::min(static_cast<size_t>(scroll_amount), image_selection.list.size() - _selection - 1);
			if (min_scroll_amount) {
				return select(_selection + U(min_scroll_amount));
			}
		} else if (scroll_amount < 0) {
			auto min_scroll_amount = std::min(static_cast<unsigned>(-scroll_amount), _selection);
			if (min_scroll_amount) {
				return select(_selection - min_scroll_amount);
			}
		}
	}
	return MakeContinueAction();
}

UiAction ImageSelection::select(unsigned index) {
	_selection = index;

	const auto& image_selection = std::get<ImageSelectionBlueprint>(blueprint->variant);
	if (!image_selection.list.empty()) {
		const auto& action_callback = image_selection.onAction;
		if (action_callback) {
			return action_callback(*this);
		}
	}
	return MakeContinueAction();
}

void ImageSelection::OnDraw() {
	draw_background_color();

	const auto& image_selection = std::get<ImageSelectionBlueprint>(blueprint->variant);
	if (!image_selection.list.empty()) {
		const auto image_rect = Rect().trim_bottom(Rect().h - Rect().w);
		DrawSpriteOrTextLabel(M2_GAME.GetSpriteOrTextLabel(image_selection.list[_selection]), image_rect);
	}

	auto buttons_rect = Rect().trim_top(Rect().w);
	auto inc_button_rect = buttons_rect.trim_left(buttons_rect.w / 2);
	sdl::render_texture_with_color_mod(_plusTexture.Texture(),
			calculate_filled_text_rect(inc_button_rect, TextHorizontalAlignment::LEFT, I(Utf8CodepointCount(_plusTexture.String().c_str()))));
	draw_border(inc_button_rect, vertical_border_width_px(), horizontal_border_width_px());

	auto dec_button_rect = buttons_rect.trim_right(buttons_rect.w / 2);
	sdl::render_texture_with_color_mod(_minusTexture.Texture(),
			calculate_filled_text_rect(dec_button_rect, TextHorizontalAlignment::LEFT, I(Utf8CodepointCount(_minusTexture.String().c_str()))));
	draw_border(dec_button_rect, vertical_border_width_px(), horizontal_border_width_px());

	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px());
}
