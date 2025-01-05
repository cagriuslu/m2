#include <m2/Game.h>
#include <m2/sdl/TextTexture.h>
#include <m2/ui/widget/Text.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::widget;

Text::Text(UiPanel* parent, const UiWidgetBlueprint* blueprint) : AbstractButton(parent, blueprint),
	_current_text(text_blueprint().text), _current_color(text_blueprint().color) {
	if (text_blueprint().on_create) {
		text_blueprint().on_create(*this);
	}
}

UiAction Text::on_update() {
	if (text_blueprint().on_update) {
		return text_blueprint().on_update(*this);
	} else {
		return MakeContinueAction();
	}
}

void Text::on_draw() {
	draw_background_color();

	// Generate font texture if necessary
	if (not _text_texture_and_destination_cache) {
		// Calculate the ideal fontSize
		const auto fontSize = text_blueprint().wrapped_font_size_in_units != 0.0f
				// Integer rounding because iround might produce too big of a font
				? RoundDownToEvenI(vertical_pixels_per_unit() * text_blueprint().wrapped_font_size_in_units)
				: calculate_filled_text_rect(drawable_area(), text_blueprint().horizontal_alignment, I(utf8_codepoint_count(_current_text.c_str()))).h;
		auto textTexture = text_blueprint().wrapped_font_size_in_units != 0.0f
				? m2_move_or_throw_error(sdl::TextTexture::create_wrapped(M2_GAME.renderer, M2_GAME.font, fontSize, drawable_area().w, text_blueprint().horizontal_alignment, _current_text))
				: m2_move_or_throw_error(sdl::TextTexture::create_nowrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, _current_text));
		auto destination_rect = text_blueprint().wrapped_font_size_in_units != 0.0f
				? calculate_wrapped_text_rect(textTexture.texture(), drawable_area(), text_blueprint().horizontal_alignment, text_blueprint().vertical_alignment)
				: calculate_filled_text_rect(drawable_area(), text_blueprint().horizontal_alignment, I(utf8_codepoint_count(_current_text.c_str())));
		_text_texture_and_destination_cache = sdl::TextTextureAndDestination{std::move(textTexture), destination_rect};
	}

	{
		// Clip to widget area
		const auto drawable_area_sdl = static_cast<SDL_Rect>(drawable_area());
		SDL_RenderSetClipRect(M2_GAME.renderer, &drawable_area_sdl);
	}
	sdl::render_texture_with_color_mod(_text_texture_and_destination_cache->textTexture.texture(),
		_text_texture_and_destination_cache->destinationRect, depressed ? _current_color / 2.0f : _current_color);
	SDL_RenderSetClipRect(M2_GAME.renderer, nullptr);

	auto border_color = depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255};
	draw_border(rect(), vertical_border_width_px(), horizontal_border_width_px(), border_color);
}

void Text::set_text(const std::string& text) {
	if (_current_text != text) {
		_current_text = text;
		_text_texture_and_destination_cache = std::nullopt;
	}
}

void Text::on_resize(const RectI& oldRect, const RectI& newRect) {
	// Check if size has changed
	if (oldRect.w != newRect.w || oldRect.h != newRect.h) {
		_text_texture_and_destination_cache = std::nullopt;
	} else if (_text_texture_and_destination_cache) {
		// Size is the same, just move the destination
		_text_texture_and_destination_cache->destinationRect.x += newRect.x - oldRect.x;
		_text_texture_and_destination_cache->destinationRect.y += newRect.y - oldRect.y;
	}
}
