#include <m2/Shape.h>
#include <m2/Exception.h>
#include <m2/sdl/Detail.h>

bool m2::ShapesSheet::ShapeKey::operator==(const ShapeKey &other) const {
	return type == other.type && color.r == other.color.r && color.g == other.color.g && color.b == other.color.b &&
			color.a == other.color.a && w == other.w && h == other.h && piece_count == other.piece_count;
}
size_t m2::ShapesSheet::ShapeKeyHash::operator()(const ShapeKey &k) const {
	uint64_t packed{};
	packed |= static_cast<uint64_t>(k.color.r ^ k.color.g);
	packed |= static_cast<uint64_t>(k.color.b ^ k.color.a) << 8;
	packed |= static_cast<uint64_t>(k.w) << 16;
	packed |= static_cast<uint64_t>(k.h) << 32;
	packed |= static_cast<uint64_t>(k.piece_count ^ k.type) << 48;
	return std::hash<uint64_t>{}(packed);
}

m2::ShapesSheet::ShapesSheet(SDL_Renderer* renderer) : DynamicTexture(renderer) {}

std::pair<SDL_Texture*, SDL_Rect> m2::ShapesSheet::get_circle(SDL_Color color, uint16_t w, uint16_t h, unsigned piece_count) {
	ShapeKey key{.type = ShapeType::CIRCLE, .color = color, .w = w, .h = h, .piece_count = (uint16_t)piece_count};

	// Check if already rendered
	auto it = _shapes.find(key);
	if (it != _shapes.end()) {
		return {texture(), it->second};
	} else {
		// Allocate space
		auto circle_rect = alloc(w, h);
		// Draw
		SDL_SetRenderTarget(renderer(), texture());
		sdl::draw_circle(renderer(), color, &circle_rect, piece_count);
		SDL_SetRenderTarget(renderer(), nullptr);
		// Save for later
		_shapes[key] = circle_rect;
		return {texture(), circle_rect};
	}
}
