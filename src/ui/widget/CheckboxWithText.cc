#include <m2/Game.h>
#include <m2/sdl/FontTexture.h>
#include <m2/ui/widget/CheckboxWithText.h>

using namespace m2::ui;
using namespace m2::ui::widget;

CheckboxWithText::CheckboxWithText(State* parent, const WidgetBlueprint* blueprint)
    : AbstractButton(parent, blueprint), _state(std::get<CheckboxWithTextBlueprint>(blueprint->variant).initial_state) {
	_font_texture = m2_move_or_throw_error(sdl::FontTexture::create_nowrap(
	    M2_GAME.font, M2_GAME.renderer, std::get<CheckboxWithTextBlueprint>(blueprint->variant).text));
}

void CheckboxWithText::on_draw() {
	// Background
	draw_background_color(rect_px, blueprint->background_color);
	// Checkbox
	auto filled_dstrect = SDL_Rect{rect_px.x, rect_px.y, rect_px.h, rect_px.h};
	SDL_SetRenderDrawColor(M2_GAME.renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(M2_GAME.renderer, &filled_dstrect);
	if (!_state) {
		auto empty_dstrect = SDL_Rect{rect_px.x + 1, rect_px.y + 1, rect_px.h - 2, rect_px.h - 2};
		SDL_SetRenderDrawColor(
		    M2_GAME.renderer, blueprint->background_color.r, blueprint->background_color.g, blueprint->background_color.b,
		    blueprint->background_color.a);
		SDL_RenderFillRect(M2_GAME.renderer, &empty_dstrect);
	}
	// Text
	if (const auto texture = _font_texture.texture(); texture) {
		auto text_rect = calculate_text_rect(rect_px.trim_left(rect_px.h), 0, 0, TextHorizontalAlignment::LEFT, texture);
		sdl::render_texture_with_color_mod(texture, text_rect);
	}
	// Border
	draw_border(rect_px, vertical_border_width_px(), horizontal_border_width_px());
}
