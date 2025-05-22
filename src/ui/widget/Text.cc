#include <m2/Game.h>
#include <m2/sdl/TextTexture.h>
#include <m2/ui/widget/Text.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::widget;

Text::Text(UiPanel* parent, const UiWidgetBlueprint* blueprint) : AbstractButton(parent, blueprint),
	_current_text(VariantBlueprint().text), _current_color(VariantBlueprint().color) {
	if (VariantBlueprint().onCreate) {
		VariantBlueprint().onCreate(*this);
	}
}

UiAction Text::OnUpdate() {
	if (VariantBlueprint().onUpdate) {
		return VariantBlueprint().onUpdate(*this);
	} else {
		return MakeContinueAction();
	}
}

void Text::OnDraw() {
	draw_background_color();

	// Generate font texture if necessary
	if (not _text_texture_and_destination_cache) {
		// Calculate the ideal fontSize
		const auto fontSize = VariantBlueprint().wrapped_font_size_in_units != 0.0f
				// Integer rounding because iround might produce too big of a font
				? RoundDownToEvenI(vertical_pixels_per_unit() * VariantBlueprint().wrapped_font_size_in_units)
				: calculate_filled_text_rect(drawable_area(), VariantBlueprint().horizontal_alignment, I(Utf8CodepointCount(_current_text.c_str()))).h;
		auto textTexture = VariantBlueprint().wrapped_font_size_in_units != 0.0f
				? m2MoveOrThrowError(sdl::TextTexture::CreateWrapped(M2_GAME.renderer, M2_GAME.font, fontSize, drawable_area().w, VariantBlueprint().horizontal_alignment, _current_text))
				: m2MoveOrThrowError(sdl::TextTexture::CreateNoWrap(M2_GAME.renderer, M2_GAME.font, M2G_PROXY.default_font_size, _current_text));
		auto destination_rect = VariantBlueprint().wrapped_font_size_in_units != 0.0f
				? calculate_wrapped_text_rect(textTexture.Texture(), drawable_area(), VariantBlueprint().horizontal_alignment, VariantBlueprint().vertical_alignment)
				: calculate_filled_text_rect(drawable_area(), VariantBlueprint().horizontal_alignment, I(Utf8CodepointCount(_current_text.c_str())));
		_text_texture_and_destination_cache = sdl::TextTextureAndDestination{std::move(textTexture), destination_rect};
	}

	{
		// Clip to widget area
		const auto drawable_area_sdl = static_cast<SDL_Rect>(drawable_area());
		SDL_RenderSetClipRect(M2_GAME.renderer, &drawable_area_sdl);
	}
	sdl::render_texture_with_color_mod(_text_texture_and_destination_cache->first.Texture(),
		_text_texture_and_destination_cache->second, depressed ? _current_color / 2.0f : _current_color);
	SDL_RenderSetClipRect(M2_GAME.renderer, nullptr);

	auto border_color = depressed ? SDL_Color{127, 127, 127, 255} : SDL_Color{255, 255, 255, 255};
	draw_border(Rect(), vertical_border_width_px(), horizontal_border_width_px(), border_color);
}

void Text::set_text(const std::string& text) {
	if (_current_text != text) {
		_current_text = text;
		_text_texture_and_destination_cache = std::nullopt;
	}
}

void Text::OnResize(const RectI& oldRect, const RectI& newRect) {
	// Check if size has changed
	if (oldRect.w != newRect.w || oldRect.h != newRect.h) {
		_text_texture_and_destination_cache = std::nullopt;
	} else if (_text_texture_and_destination_cache) {
		// Size is the same, just move the destination
		_text_texture_and_destination_cache->second.x += newRect.x - oldRect.x;
		_text_texture_and_destination_cache->second.y += newRect.y - oldRect.y;
	}
}
