#include <m2/sdl/FontTexture.h>
#include <m2/sdl/Surface.h>

namespace {
	int text_horizontal_alignment_to_ttf_wrap_alignment(m2::ui::TextHorizontalAlignment horizontal_alignment) {
		switch (horizontal_alignment) {
			case m2::ui::TextHorizontalAlignment::LEFT:
				return TTF_WRAPPED_ALIGN_LEFT;
			case m2::ui::TextHorizontalAlignment::RIGHT:
				return TTF_WRAPPED_ALIGN_RIGHT;
			default:
				return TTF_WRAPPED_ALIGN_CENTER;
		}
	}

	SDL_Texture* create_texture_with_linear_filtering(SDL_Renderer* renderer, SDL_Surface* surface) {
		// Store previous render quality
		const char* prev_render_quality = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
		// Linear filtering is less crisp, but more readable when small
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
		// Create texture
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		// Store error for later
		const char* error = nullptr;
		if (not texture) {
			error = SDL_GetError();
		}
		// Reinstate previous render quality
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, prev_render_quality);
		// Reinstate error
		if (error) {
			SDL_SetError("%s", error);
		}
		return texture;
	}
}

m2::expected<m2::sdl::FontTexture> m2::sdl::FontTexture::create_nowrap(SDL_Renderer* renderer, TTF_Font* font, int font_size, const std::string& text, SDL_Color color) {
	if (text.empty()) {
		return FontTexture{nullptr, text};
	}

	// Render to surface
	TTF_SetFontSize(font, font_size);
	SurfaceUniquePtr surface{TTF_RenderUTF8_Blended(font, text.c_str(), color)};
	m2_return_unexpected_message_unless(surface, TTF_GetError());
	// Render to texture
	SDL_Texture* texture = create_texture_with_linear_filtering(renderer, surface.get());
	m2_return_unexpected_message_unless(texture, SDL_GetError());
	return FontTexture{texture, text};
}

m2::expected<m2::sdl::FontTexture> m2::sdl::FontTexture::create_wrapped(SDL_Renderer* renderer, TTF_Font* font, int font_size,
	int font_letter_width, int width_in_chars, ui::TextHorizontalAlignment horizontal_alignment,
	const std::string& text, SDL_Color color) {
	if (text.empty()) {
		return FontTexture{nullptr, text};
	}

	// Render to surface
	TTF_SetFontSize(font, font_size);
	TTF_SetFontWrappedAlign(font, text_horizontal_alignment_to_ttf_wrap_alignment(horizontal_alignment));
	SurfaceUniquePtr surface{TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, font_letter_width * width_in_chars)};
	m2_return_unexpected_message_unless(surface, TTF_GetError());
	// Render to texture
	SDL_Texture* texture = create_texture_with_linear_filtering(renderer, surface.get());
	m2_return_unexpected_message_unless(texture, SDL_GetError());
	return FontTexture{texture, text};
}
