#include <m2/Game.h>
#include <m2/sdl/TextTexture.h>
#include <m2/ui/widget/IntegerInput.h>

using namespace m2;
using namespace m2::widget;

IntegerInput::IntegerInput(UiPanel* parent, const UiWidgetBlueprint* blueprint)
    : UiWidget(parent, blueprint), _value(std::get<IntegerInputBlueprint>(blueprint->variant).initial_value),
      _plusTexture(m2MoveOrThrowError(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, "+"))),
      _minusTexture(m2MoveOrThrowError(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, "-"))) {
	_textTexture = m2MoveOrThrowError(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, ToString(_value)));

	// Execute onCreate
	if (VariantBlueprint().onCreate) {
		auto opt_value = VariantBlueprint().onCreate(*this);
		if (opt_value) {
			// Save new value
			_value = *opt_value;
			_textTexture = m2MoveOrThrowError(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, ToString(*opt_value)));
		}
	}
}

UiAction IntegerInput::OnEvent(Events& events) {
	const auto buttonsRect = Rect().trim_left(Rect().w - Rect().h / 2);
	const auto incButtonRect = buttonsRect.trim_bottom(buttonsRect.h / 2);
	const auto decButtonRect = buttonsRect.trim_top(buttonsRect.h / 2);

	if (!_inc_depressed && events.PopMouseButtonPress(MouseButton::PRIMARY, incButtonRect)) {
		_inc_depressed = true;
		_dec_depressed = false;
	} else if (!_dec_depressed && events.PopMouseButtonPress(MouseButton::PRIMARY, decButtonRect)) {
		_dec_depressed = true;
		_inc_depressed = false;
	} else if (_inc_depressed && events.PopMouseButtonRelease(MouseButton::PRIMARY, incButtonRect)) {
		_inc_depressed = false;
		if (value() < VariantBlueprint().max_value) {
			select(value() + 1);
		}
	} else if (_dec_depressed && events.PopMouseButtonRelease(MouseButton::PRIMARY, decButtonRect)) {
		_dec_depressed = false;
		if (VariantBlueprint().min_value < value()) {
			select(value() - 1);
		}
	} else {
		// Check if scrolled
		if (const auto scrollAmount = events.PopMouseWheelVerticalScroll(Rect()); 0 < scrollAmount) {
			select(std::min(value() + scrollAmount, VariantBlueprint().max_value));
		} else if (scrollAmount < 0) {
			select(std::max(value() + scrollAmount, VariantBlueprint().min_value));
		}
	}
	return MakeContinueAction();
}

UiAction IntegerInput::select(int v) {
	_value = v;
	_textTexture = std::move(*sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, ToString(v)));

	const auto& integer_selection = std::get<IntegerInputBlueprint>(blueprint->variant);
	const auto& action_callback = integer_selection.onAction;
	if (action_callback) {
		return action_callback(*this);
	}
	return MakeContinueAction();
}

UiAction IntegerInput::OnUpdate() {
	auto& pb_blueprint = std::get<IntegerInputBlueprint>(blueprint->variant);
	if (pb_blueprint.onUpdate) {
		auto optional_value = pb_blueprint.onUpdate(*this);
		if (optional_value) {
			_value = *optional_value;
			_textTexture = std::move(*sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, ToString(*optional_value)));
		}
	}
	return MakeContinueAction();
}

void IntegerInput::OnDraw() {
	draw_background_color();

	if (const auto texture = _textTexture.texture(); texture) {
		sdl::render_texture_with_color_mod(texture,
				calculate_filled_text_rect(Rect().trim_right(Rect().h / 2), TextHorizontalAlignment::LEFT,
					I(Utf8CodepointCount(_textTexture.string().c_str()))));
	}

	auto buttons_rect = Rect().trim_left(Rect().w - Rect().h / 2);
	auto inc_button_rect = buttons_rect.trim_bottom(buttons_rect.h / 2);
	sdl::render_texture_with_color_mod(_plusTexture.texture(),
			calculate_filled_text_rect(inc_button_rect,
				TextHorizontalAlignment::LEFT, I(Utf8CodepointCount(_plusTexture.string().c_str()))));
	draw_border(inc_button_rect, vertical_border_width_px(), horizontal_border_width_px());

	auto dec_button_rect = buttons_rect.trim_top(buttons_rect.h / 2);
	sdl::render_texture_with_color_mod(_minusTexture.texture(),
			calculate_filled_text_rect(dec_button_rect,
				TextHorizontalAlignment::LEFT, I(Utf8CodepointCount(_minusTexture.string().c_str()))));
	draw_border(dec_button_rect, vertical_border_width_px(), horizontal_border_width_px());

	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px());
}

void IntegerInput::SetValue(const int value) {
	if (VariantBlueprint().min_value <= value && value < VariantBlueprint().max_value) {
		_value = value;
		if (VariantBlueprint().onAction) {
			VariantBlueprint().onAction(*this);
		}
	}
}
