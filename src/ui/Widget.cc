#include <m2/Game.h>
#include <m2/ui/Widget.h>

using namespace m2::ui;

void m2::ui::Widget::draw_background_color() const {
	const auto& color = blueprint->background_color;
	if (color.r || color.g || color.b || color.a) {
		draw_rectangle(rect(), color);
	}
}

int m2::ui::Widget::vertical_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0;
	} else {
		return std::max(1, iround(vertical_pixels_per_unit() * blueprint->border_width));
	}
}

int m2::ui::Widget::horizontal_border_width_px() const {
	if (blueprint->border_width == 0.0f) {
		return 0;
	} else {
		return std::max(1, iround(horizontal_pixels_per_unit() * blueprint->border_width));
	}
}

int m2::ui::Widget::vertical_padding_width_px() const {
	if (blueprint->padding_width == 0.0f) {
		return 0;
	} else {
		return iround(vertical_pixels_per_unit() * blueprint->padding_width);
	}
}

int m2::ui::Widget::horizontal_padding_width_px() const {
	if (blueprint->padding_width == 0.0f) {
		return 0;
	} else {
		return iround(horizontal_pixels_per_unit() * blueprint->padding_width);
	}
}

m2::RectI m2::ui::Widget::drawable_area() const {
	auto vertical_excess = vertical_border_width_px() + vertical_padding_width_px();
	auto horizontal_excess = horizontal_border_width_px() + horizontal_padding_width_px();
	return rect().trim_left(vertical_excess).trim_right(vertical_excess).trim_top(horizontal_excess).trim_bottom(horizontal_excess);
}

m2::RectI m2::ui::Widget::calculate_text_rect(SDL_Texture* text_texture, RectI drawable_area, TextHorizontalAlignment align) {
	// Fit the font into the drawable_area with correct aspect ratio
	auto text_texture_dimensions = sdl::texture_dimensions(text_texture);
	auto unaligned_destination = drawable_area.trim_to_aspect_ratio(text_texture_dimensions.x, text_texture_dimensions.y);

	// If drawable area is wider than the font, horizontal alignment is necessary.
	switch (align) {
		case TextHorizontalAlignment::LEFT:
			return unaligned_destination.align_left_to(drawable_area.x);
		case TextHorizontalAlignment::RIGHT:
			return unaligned_destination.align_right_to(drawable_area.x2());
		default:
			return unaligned_destination;
	}
}

m2::RectI m2::ui::Widget::calculate_wrapped_text_rect(SDL_Texture* text_texture, RectI drawable_area, TextHorizontalAlignment align_h, TextVerticalAlignment align_v) {
	auto text_texture_dimensions = sdl::texture_dimensions(text_texture);
	if (drawable_area.w < text_texture_dimensions.x) {
		throw M2_ERROR("Font should have been generated at most as wide as the drawable area");
	}
	RectI unaligned = RectI{-1, -1, text_texture_dimensions.x, text_texture_dimensions.y};

	RectI horizontally_aligned;
	switch (align_h) {
		case TextHorizontalAlignment::LEFT:
			horizontally_aligned = unaligned.align_left_to(drawable_area.x);
			break;
		case TextHorizontalAlignment::RIGHT:
			horizontally_aligned = unaligned.align_right_to(drawable_area.x2());
			break;
		default:
			horizontally_aligned = unaligned.align_left_to(drawable_area.x_center() - text_texture_dimensions.x / 2);
			break;
	}

	switch (align_v) {
		case TextVerticalAlignment::TOP:
			return horizontally_aligned.align_top_to(drawable_area.y);
		case TextVerticalAlignment::BOTTOM:
			return horizontally_aligned.align_bottom_to(drawable_area.y2());
		default:
			return horizontally_aligned.align_top_to(drawable_area.y_center() - text_texture_dimensions.y / 2);
	}
}

m2::RectI m2::ui::Widget::calculate_filled_text_rect(RectI drawable_area, TextHorizontalAlignment align, int text_length) {
	// Fit the font into the drawable_area with correct aspect ratio
	auto unaligned_destination = drawable_area.trim_to_aspect_ratio(
		I(text_length * M2_GAME.font_letter_width_to_height_ratio().n()),
			M2_GAME.font_letter_width_to_height_ratio().d());

	// If drawable area is wider than the font, horizontal alignment is necessary.
	switch (align) {
		case TextHorizontalAlignment::LEFT:
			return unaligned_destination.align_left_to(drawable_area.x);
		case TextHorizontalAlignment::RIGHT:
			return unaligned_destination.align_right_to(drawable_area.x2());
		default:
			return unaligned_destination;
	}
}

void Widget::draw_rectangle(const RectI& rect, const SDL_Color& color) {
	SDL_SetRenderDrawColor(M2_GAME.renderer, color.r, color.g, color.b, color.a);
	SDL_SetRenderDrawBlendMode(M2_GAME.renderer, SDL_BLENDMODE_BLEND);

	const auto sdl_rect = static_cast<SDL_Rect>(rect);
	SDL_RenderFillRect(M2_GAME.renderer, &sdl_rect);
}

void m2::ui::Widget::draw_sprite(const Sprite& sprite, const RectI& dst_rect) {
	auto src_rect = static_cast<SDL_Rect>(sprite.Rect());
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

	SDL_RenderCopy(M2_GAME.renderer, sprite.Texture(SpriteVariant{}), &src_rect, &actual_dst_rect);
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
