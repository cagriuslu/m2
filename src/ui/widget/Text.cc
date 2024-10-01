#include <m2/Game.h>
#include <m2/sdl/FontTexture.h>
#include <m2/ui/widget/Text.h>
#include <m2/Log.h>

using namespace m2::ui;
using namespace m2::ui::widget;

Text::Text(Panel* parent, const WidgetBlueprint* blueprint) : AbstractButton(parent, blueprint),
	_current_text(text_blueprint().text), _current_color(text_blueprint().color) {
	if (text_blueprint().on_create) {
		text_blueprint().on_create(*this);
	}
}

Action Text::on_update() {
	if (text_blueprint().on_update) {
		return text_blueprint().on_update(*this);
	} else {
		return make_continue_action();
	}
}

void Text::on_draw() {
	draw_background_color();

	// Generate font texture if necessary
	if (not _font_texture_and_destination_rect_cache) {
		// Calculate the ideal font_size
		auto font_size = text_blueprint().wrapped_font_size_in_units != 0.0f
			// Integer rounding because iround might produce too big of a font
			? I(vertical_pixels_per_unit() * text_blueprint().wrapped_font_size_in_units)
			: calculate_filled_text_rect(drawable_area(), text_blueprint().horizontal_alignment, I(_current_text.length())).h;

		// This clears the glyph caches, but that's the only option. As long as we don't call this every frame, it should be fine.
		// We've tried reopening the same font with different sizes, it doesn't work as expected. An invalid font is returned.
		TTF_SetFontSize(M2_GAME.font, font_size);

		auto font_texture = text_blueprint().wrapped_font_size_in_units != 0.0f
			? m2_move_or_throw_error(sdl::FontTexture::create_wrapped(M2_GAME.renderer, M2_GAME.font, drawable_area().w,
				text_blueprint().horizontal_alignment, _current_text))
			: m2_move_or_throw_error(sdl::FontTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font,
				M2G_PROXY.default_font_size, _current_text));
		auto destination_rect = text_blueprint().wrapped_font_size_in_units != 0.0f
			? calculate_wrapped_text_rect(font_texture.texture(), drawable_area(), text_blueprint().horizontal_alignment, text_blueprint().vertical_alignment)
			: calculate_filled_text_rect(drawable_area(), text_blueprint().horizontal_alignment, I(_current_text.length()));
		_font_texture_and_destination_rect_cache = std::make_tuple(std::move(font_texture), destination_rect);
	}

	{
		// Clip to widget area
		auto drawable_area_sdl = static_cast<SDL_Rect>(drawable_area());
		SDL_RenderSetClipRect(M2_GAME.renderer, &drawable_area_sdl);
	}
	sdl::render_texture_with_color_mod(std::get<sdl::FontTexture>(*_font_texture_and_destination_rect_cache).texture(),
		std::get<RectI>(*_font_texture_and_destination_rect_cache), depressed ? _current_color / 2.0f : _current_color);
	SDL_RenderSetClipRect(M2_GAME.renderer, nullptr);

	auto border_color = depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255};
	draw_border(rect(), vertical_border_width_px(), horizontal_border_width_px(), border_color);
}

void Text::set_text(const std::string& text) {
	if (_current_text != text) {
		_current_text = text;
		_font_texture_and_destination_rect_cache = std::nullopt;
	}
}
