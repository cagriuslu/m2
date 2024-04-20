#include <m2/Game.h>
#include <m2/ui/Widget.h>

using namespace m2::ui;

constexpr int DEFAULT_FONT_LETTER_WIDTH = 2;
constexpr int DEFAULT_FONT_LETTER_HEIGHT = 5;

void Widget::draw_background_color(const RectI& rect, const SDL_Color& color) {
	if (color.r || color.g || color.b || color.a) {
		SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
		SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);

		const auto sdl_rect = static_cast<SDL_Rect>(rect);
		SDL_RenderFillRect(M2_GAME.renderer, &sdl_rect);
	}
}

m2::RectI m2::ui::Widget::calculate_text_rect(
    float font_size_unitless, TextAlignment align, SDL_Texture* text_texture) const {
	return calculate_text_rect(
	    rect_px, blueprint->padding_width_px, blueprint->h, font_size_unitless, align, text_texture);
}

m2::RectI m2::ui::Widget::calculate_text_rect(
    RectI container, int padding_width_px, int container_height_unitless, float font_size_unitless, TextAlignment align,
    SDL_Texture* text_texture) {
	auto texture_dimensions = sdl::texture_dimensions(text_texture);
	// Validate font dimensions (calculations depend on it)
	if (texture_dimensions.y != 280) {
		// Current font supposed to have been rendered with 280px
		throw M2FATAL("Unexpected font height");
	}
	if ((texture_dimensions.x % 112) != 0) {
		throw M2FATAL("Unexpected font aspect ratio");
	}

	// Apply padding to container
	container = container.trim(padding_width_px);

	if (font_size_unitless == 0.0f) {
		// That's it. Fill the container with the text.
	} else {
		// Trim the container from the top and the bottom wrt given font size
		auto container_height_px_f = F(container.h);
		auto new_height_px_f = container_height_px_f * font_size_unitless / F(container_height_unitless);
		auto difference = container_height_px_f - new_height_px_f;
		auto trim_amount = I(roundf(difference / 2.0f));
		container = container.trim_top(trim_amount);
		container = container.trim_bottom(trim_amount);
	}

	// Fit the font into the container with correct aspect ratio
	int char_count = texture_dimensions.x / 112;
	// Squeeze the rect with respect to the character dimensions
	auto chars_width = char_count * DEFAULT_FONT_LETTER_WIDTH;
	auto char_height = DEFAULT_FONT_LETTER_HEIGHT;
	VecI text_dimensions = container.dimensions().aspect_ratio_dimensions(chars_width, char_height);

	RectI text_rect;
	switch (align) {
		case TextAlignment::LEFT:
			text_rect.x = container.x;
			break;
		case TextAlignment::RIGHT:
			text_rect.x = container.x + container.w - text_dimensions.x;
			break;
		default:
			text_rect.x = container.x + container.w / 2 - text_dimensions.x / 2;
			break;
	}
	text_rect.y = container.y + container.h / 2 - text_dimensions.y / 2;
	text_rect.w = text_dimensions.x;
	text_rect.h = text_dimensions.y;
	return text_rect;
}

void m2::ui::Widget::draw_text(const RectI& rect, SDL_Texture* text_texture, const RGB& color_mod) {
	// Color modulate the texture
	SDL_SetTextureColorMod(text_texture, color_mod.r, color_mod.g, color_mod.b);
	// Draw texture
	auto sdl_rect = static_cast<SDL_Rect>(rect);
	SDL_RenderCopy(M2_GAME.renderer, text_texture, nullptr, &sdl_rect);
}

void m2::ui::Widget::draw_sprite(const Sprite& sprite, const RectI& dst_rect) {
	auto src_rect = static_cast<SDL_Rect>(sprite.rect());
	auto sprite_aspect_ratio = (float)src_rect.w / (float)src_rect.h;
	auto widget_aspect_ratio = (float)dst_rect.w / (float)dst_rect.h;

	float sprite_size_multiplier =
	    sprite_aspect_ratio < widget_aspect_ratio  // Compare aspect ratios of sprite and widget
	    ? (float)dst_rect.h / (float)src_rect.h  // Widget is wider than the sprite
	    : (float)dst_rect.w / (float)src_rect.w;  // Sprite is wider than the widget

	auto actual_dst_rect = SDL_Rect{
	    .x = dst_rect.x + (dst_rect.w - iround(src_rect.w * sprite_size_multiplier)) / 2,
	    .y = dst_rect.y + (dst_rect.h - iround(src_rect.h * sprite_size_multiplier)) / 2,
	    .w = iround(src_rect.w * sprite_size_multiplier),
	    .h = iround(src_rect.h * sprite_size_multiplier)};

	SDL_RenderCopy(M2_GAME.renderer, sprite.texture(DrawVariant{}), &src_rect, &actual_dst_rect);
}

void Widget::draw_border(const RectI& rect, unsigned border_width_px, const SDL_Color& color) {
	if (border_width_px) {
		SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
		SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);

		auto sdl_rect = static_cast<SDL_Rect>(rect);
		SDL_RenderDrawRect(M2_GAME.renderer, &sdl_rect);
	}
}
