#include <m2/Game.h>
#include <m2/sdl/TextTexture.h>
#include <m2/ui/widget/CheckboxWithText.h>

using namespace m2::widget;

CheckboxWithText::CheckboxWithText(UiPanel* parent, const UiWidgetBlueprint* blueprint)
    : AbstractButton(parent, blueprint), _state(std::get<CheckboxWithTextBlueprint>(blueprint->variant).initial_state) {
	_textTexture = m2MoveOrThrowError(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font,
			M2G_PROXY.default_font_size, std::get<CheckboxWithTextBlueprint>(blueprint->variant).text)); // TODO scale the text with the window
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

void CheckboxWithText::Draw() {
	// Background
	draw_background_color();

	const auto accentColor = _state ? RGBA{127, 127, 127, 255} : RGBA{255, 255, 255, 255};

	// Text
	if (auto* texture = _textTexture.texture(); texture) {
		const auto destinationRect = calculate_filled_text_rect(Rect(), TextHorizontalAlignment::CENTER, I(Utf8CodepointCount(_textTexture.string().c_str())));
		sdl::render_texture_with_color_mod(texture, destinationRect, static_cast<RGB>(accentColor));
	}

	// Border
	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px(), accentColor);
}
