#include <m2/Game.h>
#include <m2/ui/Widget.h>

using namespace m2::ui;

constexpr int DEFAULT_FONT_LETTER_WIDTH = 2;
constexpr int DEFAULT_FONT_LETTER_HEIGHT = 5;

void m2::ui::Widget::draw_background_color() const {
	const auto& color = blueprint->background_color;
	if (color.r || color.g || color.b || color.a) {
		draw_rectangle(rect_px, color);
	}
}

int m2::ui::Widget::vertical_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0;
	} else {
		float pixel_pitch = F(rect_px.w) / F(blueprint->w); // Pixels per unit
		return std::max(1, iround(pixel_pitch * blueprint->border_width));
	}
}

int m2::ui::Widget::horizontal_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0;
	} else {
		float pixel_pitch = F(rect_px.h) / F(blueprint->h); // Pixels per unit
		return std::max(1, iround(pixel_pitch * blueprint->border_width));
	}
}

int m2::ui::Widget::vertical_padding_width_px() const {
	if (blueprint->padding_width == 0.0f) {
		return 0;
	} else {
		float pixel_pitch = F(rect_px.w) / F(blueprint->w); // Pixels per unit
		return iround(pixel_pitch * blueprint->padding_width);
	}
}

int m2::ui::Widget::horizontal_padding_width_px() const {
	if (blueprint->padding_width == 0.0f) {
		return 0;
	} else {
		float pixel_pitch = F(rect_px.h) / F(blueprint->h); // Pixels per unit
		return iround(pixel_pitch * blueprint->padding_width);
	}
}

m2::RectI m2::ui::Widget::drawable_area() const {
	auto vertical_excess = vertical_border_width_px() + vertical_padding_width_px();
	auto horizontal_excess = horizontal_border_width_px() + horizontal_padding_width_px();
	return rect_px.trim_left(vertical_excess).trim_right(vertical_excess).trim_top(horizontal_excess).trim_bottom(horizontal_excess);
}

m2::RectI m2::ui::Widget::calculate_text_rect(float font_size_unitless, TextHorizontalAlignment align,
	SDL_Texture* text_texture) const {
	return calculate_text_rect(rect_px, blueprint->h, font_size_unitless, align, text_texture);
}

m2::RectI m2::ui::Widget::calculate_text_rect(RectI container, int container_height_unitless, float font_size_unitless,
	TextHorizontalAlignment align, SDL_Texture* text_texture) {
	auto texture_dimensions = sdl::texture_dimensions(text_texture);
	// Validate font dimensions (calculations depend on it)
	if (texture_dimensions.y != 280) {
		// Current font supposed to have been rendered with 280px
		throw M2_ERROR("Unexpected font height");
	}
	if ((texture_dimensions.x % 112) != 0) {
		throw M2_ERROR("Unexpected font aspect ratio");
	}

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
		case TextHorizontalAlignment::LEFT:
			text_rect.x = container.x;
			break;
		case TextHorizontalAlignment::RIGHT:
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

void Widget::draw_rectangle(const m2::RectI& rect, const SDL_Color& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);

	const auto sdl_rect = static_cast<SDL_Rect>(rect);
	SDL_RenderFillRect(M2_GAME.renderer, &sdl_rect);
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

void Widget::draw_border(const RectI& rect, int vertical_border_width_px, int horizontal_border_width_px, const SDL_Color& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);

	SDL_Rect left_right_top_bottom[4] = {
		{.x = rect.x, .y = rect.y, .w = vertical_border_width_px, .h = rect.h},
		{.x = rect.x2() - vertical_border_width_px, .y = rect.y, .w = vertical_border_width_px, .h = rect.h},
		{.x = rect.x, .y = rect.y, .w = rect.w, .h = horizontal_border_width_px},
		{.x = rect.x, .y = rect.y2() - horizontal_border_width_px, .w = rect.w, .h = horizontal_border_width_px}
	};
	SDL_RenderFillRects(M2_GAME.renderer, left_right_top_bottom, 4);
}
