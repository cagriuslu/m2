#include <m2/Shape.h>
#include <m2/Exception.h>
#include <m2/sdl/Utils.hh>

bool m2::ShapesSheet::ShapeKey::operator==(const ShapeKey &other) const {
	return type == other.type && color.r == other.color.r && color.g == other.color.g && color.b == other.color.b &&
			color.a == other.color.a && x1 == other.x1 && y1 == other.y1 && w == other.w && h == other.h && r == other.r;
}
size_t m2::ShapesSheet::ShapeKeyHash::operator()(const ShapeKey &k) const {
	uint64_t packed{};
	packed |= static_cast<uint64_t>(k.color.r ^ k.color.g);
	packed |= static_cast<uint64_t>(k.color.b ^ k.color.a) << 8;
	packed |= static_cast<uint64_t>(k.x1) << 16;
	packed |= static_cast<uint64_t>(k.y1) << 24;
	packed |= static_cast<uint64_t>(k.w) << 32;
	packed |= static_cast<uint64_t>(k.h) << 40;
	packed |= static_cast<uint64_t>(k.r) << 48;
	packed |= static_cast<uint64_t>(k.type) << 56;
	return std::hash<uint64_t>{}(packed);
}

m2::ShapesSheet::ShapesSheet(SDL_Renderer* renderer) : DynamicSheet(renderer) {}

std::pair<SDL_Texture*, SDL_Rect> m2::ShapesSheet::get_pixel(SDL_Color color) {
	// Check if already rendered
	ShapeKey key{.type = ShapeType::PIXEL, .color = color};
	auto it = _shapes.find(key);
	if (it != _shapes.end()) {
		return {texture(), it->second};
	} else {
		// Allocate space from the dynamic sheet
		auto [surface, rect] = alloc(1, 1);
		// Lock surface
		if (SDL_LockSurface(surface)) {
			throw M2ERROR("Unable to lock surface: " + std::string{SDL_GetError()});
		}
		// Draw pixel
		auto pixel = SDL_MapRGBA(surface->format, color.r, color.g, color.b, color.a);
		sdl::set_pixel(surface, rect.x, rect.y, pixel);
		// Unlock surface
		SDL_UnlockSurface(surface);
		// Save to map
		_shapes[key] = rect;
		return {recreate_texture(), rect};
	}
}
std::pair<SDL_Texture*, SDL_Rect> m2::ShapesSheet::get_line(SDL_Color color, int x1, int y1) {
	ShapeKey key{.type = ShapeType::LINE, .color = color, .x1 = x1, .y1 = y1};

	// Check if already rendered
	auto it = _shapes.find(key);
	if (it != _shapes.end()) {
		return {texture(), it->second};
	} else {
		int x0 = 0;
		int y0 = 0;
		int diff_x = abs(x1 - x0);
		int diff_y = abs(y1 - y0);
		int step_x = x0 < x1 ? 1 : -1;
		int step_y = y0 < y1 ? 1 : -1;
		int w = diff_x + 1;
		int h = diff_y + 1;

		auto [line_surface, line_rect] = alloc(w, h);
		if (SDL_LockSurface(line_surface)) {
			throw M2ERROR("Unable to lock surface: " + std::string{SDL_GetError()});
		}
		auto pixel = SDL_MapRGBA(line_surface->format, color.r, color.g, color.b, color.a);

		int error = diff_x - diff_y;
		while (true) {
			sdl::set_pixel(line_surface, x0 + line_rect.x, y0 + line_rect.y, pixel);
			if (x0 == x1 && y0 == y1) {
				break;
			}

			int error2 = error * 2;
			if (-diff_y <= error2) {
				error -= diff_y;
				x0 += step_x;
			}
			if (error2 <= diff_x) {
				error += diff_x;
				y0 += step_y;
			}
		}

		SDL_UnlockSurface(line_surface);

		_shapes[key] = line_rect;
		return {recreate_texture(), line_rect};
	}
}
std::pair<SDL_Texture*, SDL_Rect> m2::ShapesSheet::get_rectangle_aa(SDL_Color color, int w, int h) {
	ShapeKey key{.type = ShapeType::RECTANGLE_AA, .color = color, .w = w, .h = h};

	// Check if already rendered
	auto it = _shapes.find(key);
	if (it != _shapes.end()) {
		return {texture(), it->second};
	} else {
		auto [rectangle_surface, rectangle_rect] = alloc(w, h);
		if (SDL_LockSurface(rectangle_surface)) {
			throw M2ERROR("Unable to lock surface: " + std::string{SDL_GetError()});
		}
		auto pixel = SDL_MapRGBA(rectangle_surface->format, color.r, color.g, color.b, color.a);

		// Top
		for (int x = 0; x < w; ++x) {
			sdl::set_pixel(rectangle_surface, x + rectangle_rect.x, rectangle_rect.y, pixel);
		}
		// Left
		for (int y = 0; y < h; ++y) {
			sdl::set_pixel(rectangle_surface, rectangle_rect.x, y + rectangle_rect.y, pixel);
		}
		// Right
		for (int y = 0; y < h; ++y) {
			sdl::set_pixel(rectangle_surface, rectangle_rect.x + w - 1, y + rectangle_rect.y, pixel);
		}
		// Bottom
		for (int x = 0; x < w; ++x) {
			sdl::set_pixel(rectangle_surface, x + rectangle_rect.x, rectangle_rect.y + h - 1, pixel);
		}

		SDL_UnlockSurface(rectangle_surface);

		_shapes[key] = rectangle_rect;
		return {recreate_texture(), rectangle_rect};
	}
}
std::pair<SDL_Texture*, SDL_Rect> m2::ShapesSheet::get_circle(SDL_Color color, int r) {
	int w = r * 2 + 1;
	ShapeKey key{.type = ShapeType::CIRCLE, .color = color, .r = r};

	// Check if already rendered
	auto it = _shapes.find(key);
	if (it != _shapes.end()) {
		return {texture(), it->second};
	} else {
		int middle_x = r, middle_y = r;

		auto [circle_surface, circle_rect] = alloc(w, w);
		if (SDL_LockSurface(circle_surface)) {
			throw M2ERROR("Unable to lock surface: " + std::string{SDL_GetError()});
		}
		auto pixel = SDL_MapRGBA(circle_surface->format, color.r, color.g, color.b, color.a);

		int x = -r, y = 0, err = 2 - 2 * r;
		do {
			sdl::set_pixel(circle_surface, middle_x - x + circle_rect.x, middle_y + y + circle_rect.y, pixel);
			sdl::set_pixel(circle_surface, middle_x - y + circle_rect.x, middle_y - x + circle_rect.y, pixel);
			sdl::set_pixel(circle_surface, middle_x + x + circle_rect.x, middle_y - y + circle_rect.y, pixel);
			sdl::set_pixel(circle_surface, middle_x + y + circle_rect.x, middle_y + x + circle_rect.y, pixel);
			r = err;
			if (r <= y) {
				err += ++y * 2 + 1;
			}
			if (r > x || err > y) {
				err += ++x * 2 + 1;
			}
		} while (x < 0);

		SDL_UnlockSurface(circle_surface);

		_shapes[key] = circle_rect;
		return {recreate_texture(), circle_rect};
	}
}
