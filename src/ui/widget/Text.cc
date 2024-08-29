#include <m2/Game.h>
#include <m2/sdl/FontTexture.h>
#include <m2/ui/widget/Text.h>

using namespace m2::ui;
using namespace m2::ui::widget;

Text::Text(State* parent, const WidgetBlueprint* blueprint) : AbstractButton(parent, blueprint) {
	auto initial_text = text_blueprint().text;
	_font_texture = m2_move_or_throw_error(sdl::FontTexture::create_nowrap(M2_GAME.font, M2_GAME.renderer, initial_text));
	_color_override = text_blueprint().color;

	if (text_blueprint().on_create) {
		text_blueprint().on_create(*this);
	}
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
		return text_blueprint().on_update(*this);
	} else {
		return make_continue_action();
	}
}

void Text::on_draw() {
	draw_background_color(rect_px, blueprint->background_color);
	if (const auto texture = _font_texture.texture(); texture) {
		sdl::render_texture_with_color_mod(texture,
			calculate_text_rect(text_blueprint().font_size, text_blueprint().horizontal_alignment, texture),
		    depressed ? _color_override / 2.0f : _color_override);
	}
	auto border_color = depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255};
	draw_border(rect_px, vertical_border_width_px(), horizontal_border_width_px(), border_color);
}

void Text::set_text(const std::string& t) {
	_font_texture = m2_move_or_throw_error(sdl::FontTexture::create_nowrap(M2_GAME.font, M2_GAME.renderer, t));
}

void Text::set_color(RGB&& c) {
	_color_override = c;
}
