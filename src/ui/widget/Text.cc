#include <m2/ui/widget/Text.h>
#include <m2/sdl/Font.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;

Text::Text(State* parent, const WidgetBlueprint* blueprint) : AbstractButton(parent, blueprint) {
	const auto inital_text = std::get<TextBlueprint>(blueprint->variant).initial_text;
	_font_texture = m2_move_or_throw_error(sdl::FontTexture::create(inital_text));
}

Action Text::on_update() {
	if (disable_after) {
		*disable_after -= GAME.delta_time_s();
		if (*disable_after <= 0.0f) {
			disable_after = {};
			enabled = false;
			return Action::CONTINUE;
		}
	}

	auto& text_blueprint = std::get<TextBlueprint>(blueprint->variant);
	if (text_blueprint.on_update) {
		auto[action, optional_string] = text_blueprint.on_update(*this);
		if (action == Action::CONTINUE && optional_string) {
			_font_texture = m2_move_or_throw_error(sdl::FontTexture::create(*optional_string));
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}

void Text::on_draw() {
	draw_background_color(rect_px, blueprint->background_color);
	if (const auto texture = _font_texture.texture(); texture) {
		if (depressed) {
			SDL_SetTextureColorMod(texture, 127, 127, 127);
		} else {
			SDL_SetTextureColorMod(texture, 255, 255, 255);
		}
		auto text_rect = rect_px.trim(blueprint->padding_width_px);
		draw_text(text_rect, *texture, std::get<TextBlueprint>(blueprint->variant).alignment);
	}
	draw_border(rect_px, blueprint->border_width_px, depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255});
}
