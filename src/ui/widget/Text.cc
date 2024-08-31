#include <m2/Game.h>
#include <m2/sdl/FontTexture.h>
#include <m2/ui/widget/Text.h>

using namespace m2::ui;
using namespace m2::ui::widget;

Text::Text(State* parent, const WidgetBlueprint* blueprint) : AbstractButton(parent, blueprint) {
	// During construction, the rect_px of the widget is not yet determined. Font texture should be generated later.
	set_text(text_blueprint().text);
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
	draw_background_color();

	// Generate font texture if necessary
	if (_string != _font_texture.string()) {
		if (text_blueprint().wrapped_font_size_in_units != 0.0f) {
			_font_texture = m2_move_or_throw_error(sdl::FontTexture::create_wrapped(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size,
				M2G_PROXY.default_font_letter_width, widget_width_in_chars(), text_blueprint().horizontal_alignment, _string));
		} else {
			_font_texture = m2_move_or_throw_error(sdl::FontTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, _string));
		}
	}

	if (const auto texture = _font_texture.texture()) {
		{
			// Clip to widget area
			auto drawable_area_sdl = static_cast<SDL_Rect>(drawable_area());
			SDL_RenderSetClipRect(M2_GAME.renderer, &drawable_area_sdl);
		}

		if (text_blueprint().wrapped_font_size_in_units != 0.0f) {
			// Find the ratio at which a font letter width becomes widget letter width
			auto squeeze_ratio = F(M2G_PROXY.default_font_letter_width) / widget_letter_width_in_pixels();
			// TODO implement vertical alignment
			auto destination = RectI{
				rect_px.x + vertical_border_width_px(),
				rect_px.y + horizontal_border_width_px(),
				iround(F(sdl::texture_dimensions(texture).x) / squeeze_ratio),
				iround(F(sdl::texture_dimensions(texture).y) / squeeze_ratio)
			};
			sdl::render_texture_with_color_mod(texture, destination,
				depressed ? _color_override / 2.0f : _color_override);
		} else {
			sdl::render_texture_with_color_mod(texture,
				calculate_text_rect(texture, drawable_area(), text_blueprint().horizontal_alignment),
				depressed ? _color_override / 2.0f : _color_override);
		}

		// Unclip
		SDL_RenderSetClipRect(M2_GAME.renderer, nullptr);
	}
	auto border_color = depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255};
	draw_border(rect_px, vertical_border_width_px(), horizontal_border_width_px(), border_color);
}

void Text::set_text(const std::string& text) {
	_string = text;
	_font_texture = {};
}

void Text::set_color(RGB&& c) {
	_color_override = c;
}

int Text::widget_width_in_chars() const {
	// This function shouldn't be called if font size is 0.
	if (text_blueprint().wrapped_font_size_in_units == 0.0f) {
		throw M2_ERROR("Font size is 0");
	}

	// Calculate how many pixels there are per horizontal 'unit'
	auto horizontal_pixels_per_unit = F(rect_px.w) / F(blueprint->w);
	// Calculate the width of the drawable area in pixels
	auto max_text_width_px = rect_px.w - vertical_border_width_px() - vertical_border_width_px();
	// Calculate how many 'units' there are in the horizontal drawable area
	auto max_text_width_in_units = F(max_text_width_px) / horizontal_pixels_per_unit;
	// Calculate a letter's width in 'units'
	auto font_letter_width_in_units = text_blueprint().wrapped_font_size_in_units * M2G_PROXY.default_font_letter_width / M2G_PROXY.default_font_size;
	// Calculate how many letters fit in the horizontal drawable area
	return I(max_text_width_in_units / font_letter_width_in_units);
}

float Text::widget_letter_width_in_pixels() const {
	// Calculate how many pixels there are per horizontal 'unit'
	auto horizontal_pixels_per_unit = F(rect_px.w) / F(blueprint->w);
	// Calculate a letter's width in 'units'
	auto font_letter_width_in_units = text_blueprint().wrapped_font_size_in_units * M2G_PROXY.default_font_letter_width / M2G_PROXY.default_font_size;
	return font_letter_width_in_units * horizontal_pixels_per_unit;
}
