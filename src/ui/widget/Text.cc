#include <m2/ui/widget/Text.h>
#include <m2/Game.h>

using namespace m2::ui;
using namespace m2::ui::widget;

Text::Text(const WidgetBlueprint* blueprint) : AbstractButton(blueprint), font_texture(sdl::generate_font(std::get<TextBlueprint>(blueprint->variant).initial_text)) {}

Action Text::update_content() {
	if (disable_after) {
		*disable_after -= GAME.delta_time_s();
		if (*disable_after <= 0.0f) {
			disable_after = {};
			enabled = false;
			return Action::CONTINUE;
		}
	}

	auto& text_blueprint = std::get<TextBlueprint>(blueprint->variant);
	if (text_blueprint.update_callback) {
		auto[action, optional_string] = text_blueprint.update_callback();
		if (action == Action::CONTINUE && optional_string) {
			font_texture = sdl::generate_font(*optional_string);
		}
		return action;
	} else {
		return Action::CONTINUE;
	}
}

void Text::draw() {
	draw_background_color(rect_px, blueprint->background_color);
	if (font_texture) {
		if (depressed) {
			SDL_SetTextureColorMod(font_texture.get(), 127, 127, 127);
		} else {
			SDL_SetTextureColorMod(font_texture.get(), 255, 255, 255);
		}
		draw_text(sdl::expand_rect(rect_px, -static_cast<int>(blueprint->padding_width_px)), *font_texture, std::get<TextBlueprint>(blueprint->variant).alignment);
	}
	draw_border(rect_px, blueprint->border_width_px, depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255});
}
