#include <m2/Game.h>
#include <m2/sdl/TextTexture.h>
#include <m2/ui/widget/IntegerSelection.h>

using namespace m2;
using namespace m2::widget;

IntegerSelection::IntegerSelection(UiPanel* parent, const UiWidgetBlueprint* blueprint)
    : UiWidget(parent, blueprint), _value(std::get<IntegerSelectionBlueprint>(blueprint->variant).initial_value) {
	if (VariantBlueprint().onCreate) {
		if (const auto value = VariantBlueprint().onCreate(*this)) {
			_value = *value;
		}
	}
}

UiAction IntegerSelection::SetValue(const int value) {
	if (VariantBlueprint().min_value <= value && value < VariantBlueprint().max_value) {
		_value = value;
		_textTexture = sdl::TextTextureAndDestination{};
		if (VariantBlueprint().onAction) {
			return VariantBlueprint().onAction(*this);
		}
	}
	return MakeContinueAction();
}

void IntegerSelection::OnResize(const RectI&, const RectI&) {
	_textTexture = sdl::TextTextureAndDestination{};
	_plusTexture = sdl::TextTextureAndDestination{};
	_minusTexture = sdl::TextTextureAndDestination{};
}
UiAction IntegerSelection::OnEvent(Events& events) {
	const auto [plusArea, minusArea] = CalculatePlusAndMinusButtonDrawArea();
	if (!_inc_depressed && events.PopMouseButtonPress(MouseButton::PRIMARY, plusArea)) {
		_inc_depressed = true;
		_dec_depressed = false;
	} else if (!_dec_depressed && events.PopMouseButtonPress(MouseButton::PRIMARY, minusArea)) {
		_dec_depressed = true;
		_inc_depressed = false;
	} else if (_inc_depressed && events.PopMouseButtonRelease(MouseButton::PRIMARY, plusArea)) {
		_inc_depressed = false;
		if (value() < VariantBlueprint().max_value) {
			return SetValue(value() + 1);
		}
	} else if (_dec_depressed && events.PopMouseButtonRelease(MouseButton::PRIMARY, minusArea)) {
		_dec_depressed = false;
		if (VariantBlueprint().min_value < value()) {
			return SetValue(value() - 1);
		}
	} else {
		// Check if scrolled
		if (const auto scrollAmount = events.PopMouseWheelVerticalScroll(Rect()); 0 < scrollAmount) {
			return SetValue(std::min(value() + scrollAmount, VariantBlueprint().max_value));
		} else if (scrollAmount < 0) {
			return SetValue(std::max(value() + scrollAmount, VariantBlueprint().min_value));
		}
	}
	return MakeContinueAction();
}
UiAction IntegerSelection::OnUpdate() {
	if (auto& pb_blueprint = std::get<IntegerSelectionBlueprint>(blueprint->variant); pb_blueprint.onUpdate) {
		if (const auto value = pb_blueprint.onUpdate(*this)) {
			return SetValue(*value);
		}
	}
	return MakeContinueAction();
}

void IntegerSelection::OnDraw() {
	draw_background_color();

	{
		// Draw the integer value
		if (not _textTexture.first) {
			const auto valueAsString = ToString(_value);
			_textTexture.second = calculate_filled_text_rect(Rect().trim_right(Rect().h / 2), TextHorizontalAlignment::LEFT, valueAsString.size());
			_textTexture.first = m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, _textTexture.second.h, valueAsString));
		}
		sdl::render_texture_with_color_mod(_textTexture.first.Texture(), _textTexture.second);
	}

	const auto [plusArea, minusArea] = CalculatePlusAndMinusButtonDrawArea();
	{
		// Draw plus texture
		if (not _plusTexture.first) {
			_plusTexture.second = calculate_filled_text_rect(plusArea, TextHorizontalAlignment::CENTER, 1);
			_plusTexture.first = m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, _plusTexture.second.h, "+"));
		}
		sdl::render_texture_with_color_mod(_plusTexture.first.Texture(), _plusTexture.second);
		draw_border(plusArea, vertical_border_width_px(), horizontal_border_width_px());
	}
	{
		// Draw minus texture
		if (not _minusTexture.first) {
			_minusTexture.second = calculate_filled_text_rect(minusArea, TextHorizontalAlignment::CENTER, 1);
			_minusTexture.first = m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, _minusTexture.second.h, "-"));
		}
		sdl::render_texture_with_color_mod(_minusTexture.first.Texture(), _minusTexture.second);
		draw_border(minusArea, vertical_border_width_px(), horizontal_border_width_px());
	}

	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px());
}

RectI IntegerSelection::CalculateValueDrawArea() const {
	return Rect().trim_right(Rect().h / 2);
}
std::pair<RectI,RectI> IntegerSelection::CalculatePlusAndMinusButtonDrawArea() const {
	const auto buttonsRect = Rect().trim_left(Rect().w - Rect().h / 2);
	return std::make_pair(buttonsRect.trim_bottom(buttonsRect.h / 2), buttonsRect.trim_top(buttonsRect.h / 2));
}
