#include <m2/Game.h>
#include <m2/sdl/Font.h>
#include <m2/ui/widget/Text.h>

using namespace m2::ui;
using namespace m2::ui::widget;

Text::Text(State* parent, const WidgetBlueprint* blueprint) : AbstractButton(parent, blueprint) {
	auto initial_text = text_blueprint().initial_text;

	// Execute on_create
	if (text_blueprint().on_create) {
		auto opt_string = text_blueprint().on_create(*this);
		if (opt_string) {
			// Overwrite initial_text
			initial_text = *opt_string;
		}
	}

	_font_texture = m2_move_or_throw_error(sdl::FontTexture::create(M2_GAME.font, M2_GAME.renderer, initial_text));
}

Action Text::on_update() {
	if (disable_after) {
		*disable_after -= M2_GAME.delta_time_s();
		if (*disable_after <= 0.0f) {
			disable_after = {};
			enabled = false;
			return make_continue_action();
		}
	}

	if (text_blueprint().on_update) {
		auto [action, optional_string] = text_blueprint().on_update(*this);
		if (action.is_continue() && optional_string) {
			_font_texture =
			    m2_move_or_throw_error(sdl::FontTexture::create(M2_GAME.font, M2_GAME.renderer, *optional_string));
		}
		return std::move(action);
	} else {
		return make_continue_action();
	}
}

void Text::on_draw() {
	draw_background_color(rect_px, blueprint->background_color);
	if (const auto texture = _font_texture.texture(); texture) {
		draw_text(
		    calculate_text_rect(text_blueprint().font_size, text_blueprint().alignment, texture), texture,
		    depressed ? RGB{127, 127, 127} : RGB{255, 255, 255});
	}
	draw_border(
	    rect_px, blueprint->border_width_px, depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255});
}
