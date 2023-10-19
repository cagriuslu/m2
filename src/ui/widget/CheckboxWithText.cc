#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;

CheckboxWithText::CheckboxWithText(State* parent, const WidgetBlueprint *blueprint) : AbstractButton(parent, blueprint), _state(std::get<CheckboxWithTextBlueprint>(blueprint->variant).initial_state), _font_texture(sdl::generate_font(std::get<CheckboxWithTextBlueprint>(blueprint->variant).text)) {}

void CheckboxWithText::on_draw() {
	// Background
	draw_background_color(rect_px, blueprint->background_color);
	// Checkbox
	auto filled_dstrect = SDL_Rect{rect_px.x, rect_px.y, rect_px.h, rect_px.h};
	SDL_SetRenderDrawColor(GAME.renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(GAME.renderer, &filled_dstrect);
	if (!_state) {
		auto empty_dstrect = SDL_Rect{rect_px.x + 1, rect_px.y + 1, rect_px.h - 2, rect_px.h - 2};
		SDL_SetRenderDrawColor(GAME.renderer, blueprint->background_color.r, blueprint->background_color.g, blueprint->background_color.b, blueprint->background_color.a);
		SDL_RenderFillRect(GAME.renderer, &empty_dstrect);
	}
	// Text
	if (_font_texture) {
		auto text_rect = RectI{rect_px};
		draw_text((SDL_Rect)text_rect.trim_left(rect_px.h).trim((int)blueprint->padding_width_px), *_font_texture, TextAlignment::LEFT);
	}
	// Border
	draw_border(rect_px, blueprint->border_width_px);
}
