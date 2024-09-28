#include <m2/Game.h>
#include <m2/sdl/FontTexture.h>
#include <m2/ui/widget/CheckboxWithText.h>

using namespace m2::ui;
using namespace m2::ui::widget;

CheckboxWithText::CheckboxWithText(Panel* parent, const WidgetBlueprint* blueprint)
    : AbstractButton(parent, blueprint), _state(std::get<CheckboxWithTextBlueprint>(blueprint->variant).initial_state) {
	_font_texture = m2_move_or_throw_error(sdl::FontTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size,
		std::get<CheckboxWithTextBlueprint>(blueprint->variant).text));
}

void CheckboxWithText::on_draw() {
	// Background
	draw_background_color();
	// Checkbox
	auto filled_dstrect = SDL_Rect{rect().x, rect().y, rect().h, rect().h};
	SDL_SetRenderDrawColor(M2_GAME.renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(M2_GAME.renderer, &filled_dstrect);
	if (!_state) {
		auto empty_dstrect = SDL_Rect{rect().x + 1, rect().y + 1, rect().h - 2, rect().h - 2};
		SDL_SetRenderDrawColor(
		    M2_GAME.renderer, blueprint->background_color.r, blueprint->background_color.g, blueprint->background_color.b,
		    blueprint->background_color.a);
		SDL_RenderFillRect(M2_GAME.renderer, &empty_dstrect);
	}
	// Text
	if (const auto texture = _font_texture.texture(); texture) {
		auto text_rect = calculate_text_rect(texture, rect().trim_left(rect().h), TextHorizontalAlignment::LEFT);
		sdl::render_texture_with_color_mod(texture, text_rect);
	}
	// Border
	draw_border(rect(), vertical_border_width_px(), horizontal_border_width_px());
}
