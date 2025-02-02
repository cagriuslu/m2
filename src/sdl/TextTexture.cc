#include <m2/sdl/TextTexture.h>
#include <m2/sdl/Surface.h>

namespace {
	int ToTtfWrapAlignment(m2::TextHorizontalAlignment horizontal_alignment) {
		switch (horizontal_alignment) {
			case m2::TextHorizontalAlignment::LEFT:
				return TTF_WRAPPED_ALIGN_LEFT;
			case m2::TextHorizontalAlignment::RIGHT:
				return TTF_WRAPPED_ALIGN_RIGHT;
			default:
				return TTF_WRAPPED_ALIGN_CENTER;
		}
	}

	SDL_Texture* create_texture_with_linear_filtering(SDL_Renderer* renderer, SDL_Surface* surface) {
		// Store previous render quality
		const char* prev_render_quality_ptr = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);
		const auto prevRenderScaleQuality = prev_render_quality_ptr ? std::string{prev_render_quality_ptr} : std::string{};
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
		if (not prevRenderScaleQuality.empty()) {
			SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, prevRenderScaleQuality.c_str());
		}
		// Reinstate error
		if (error) {
			SDL_SetError("%s", error);
		}
		return texture;
	}
}

m2::expected<m2::sdl::TextTexture> m2::sdl::TextTexture::create_nowrap(SDL_Renderer* renderer, TTF_Font* font, int fontSize, const std::string& text, SDL_Color color) {
	if (text.empty()) {
		return TextTexture{nullptr, text};
	}

	// This clears the glyph caches, but that's the only option we have. As long as we don't call this every frame, it
	// should be fine.
	TTF_SetFontSize(font, fontSize);
	// Render to surface
	SurfaceUniquePtr surface{TTF_RenderUTF8_Blended(font, text.c_str(), color)};
	m2_return_unexpected_message_unless(surface, TTF_GetError());
	// Render to texture
	SDL_Texture* texture = create_texture_with_linear_filtering(renderer, surface.get());
	m2_return_unexpected_message_unless(texture, SDL_GetError());
	return TextTexture{texture, text};
}

m2::expected<m2::sdl::TextTexture> m2::sdl::TextTexture::create_wrapped(SDL_Renderer* renderer, TTF_Font* font, int fontSize, int width_px, TextHorizontalAlignment horizontal_alignment, const std::string& text, SDL_Color color) {
	if (text.empty()) {
		return TextTexture{nullptr, text};
	}

	// This clears the glyph caches, but that's the only option we have. As long as we don't call this every frame, it
	// should be fine.
	TTF_SetFontSize(font, fontSize);
	// Render to surface
	TTF_SetFontWrappedAlign(font, ToTtfWrapAlignment(horizontal_alignment));
	SurfaceUniquePtr surface{TTF_RenderUTF8_Blended_Wrapped(font, text.c_str(), color, width_px)};
	m2_return_unexpected_message_unless(surface, TTF_GetError());
	// Render to texture
	SDL_Texture* texture = create_texture_with_linear_filtering(renderer, surface.get());
	m2_return_unexpected_message_unless(texture, SDL_GetError());
	return TextTexture{texture, text};
}
