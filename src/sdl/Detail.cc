#include <m2/sdl/Detail.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/M2.h>

SDL_Cursor* SdlUtils_CreateCursor() {
    const char* str =
		"                                "
		"                                "
		"             XXXXX              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             XXXXX              "
		"                                "
		"  XXXXXXXXXX XXXXX XXXXXXXXXX   "
		"  X........X X...X X........X   "
		"  X........X X...X X........X   "
		"  X........X X...X X........X   "
		"  XXXXXXXXXX XXXXX XXXXXXXXXX   "
		"                                "
		"             XXXXX              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             X...X              "
		"             XXXXX              "
		"                                "
		"                                "
		"                                "
	;

    uint8_t data[128] = { 0 };
    uint8_t mask[128] = { 0 };
    const unsigned side_size = 32;

	for (unsigned y = 0; y < side_size; y++) {
		for (unsigned x = 0; x < side_size; x++) {
            unsigned index = y * side_size + x;
            char c = str[index];
			if (c == 'X') {
                data[index / 8] |= 0x80 >> (index % 8);
                mask[index / 8] |= 0x80 >> (index % 8);
			} else if (c == '.') {
                mask[index / 8] |= 0x80 >> (index % 8);
			}
		}
	}

	return SDL_CreateCursor(data, mask, side_size, side_size, side_size / 2 - 1, side_size / 2 - 1);
}

void m2::sdl::delay(ticks_t duration) {
	if (0 < duration) {
		SDL_Delay(duration);
	}
}

m2::sdl::ticks_t m2::sdl::get_ticks() {
	return static_cast<int64_t>(SDL_GetTicks64()) / time_slowdown_factor;
}

m2::sdl::ticks_t m2::sdl::get_ticks_since(ticks_t last_ticks, ticks_t pause_ticks) {
	return get_ticks() - last_ticks - pause_ticks;
}

int m2::sdl::get_refresh_rate() {
	SDL_DisplayMode dm{};
	SDL_GetWindowDisplayMode(M2_GAME.window, &dm);
	return dm.refresh_rate;
}

int m2::sdl::draw_circle(SDL_Renderer* renderer, SDL_Color color, SDL_Rect* dst_rect, unsigned piece_count) {
	std::vector<SDL_Point> points{piece_count + 1};
	for (unsigned i = 0; i < piece_count; ++i) {
		int x = (int) std::roundf(std::cosf(m2::PI_MUL2 * (float)i / (float)piece_count) * (float)dst_rect->w / 2.0f);
		int y = (int) std::roundf(std::sinf(m2::PI_MUL2 * (float)i / (float)piece_count) * (float)dst_rect->h / 2.0f);
		points[i].x = dst_rect->x + dst_rect->w / 2 + x;
		points[i].y = dst_rect->y + dst_rect->h / 2 + y;
	}
	points[piece_count] = points[0]; // Last point

	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
	return SDL_RenderDrawLines(renderer, points.data(), (int) points.size());
}

int m2::sdl::draw_disk(SDL_Renderer* renderer, const VecF& center_position_px, const SDL_Color& center_color, float radius_px, const SDL_Color& edge_color, unsigned steps) {
	std::vector<SDL_Vertex> vertices(steps * 3);
	// The vector that'll be rotated for the edges
	VecF full_span_px{radius_px, 0.0f};
	for (unsigned i = 0; i < steps; ++i) {
		// Center point of the triangle
		vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(center_position_px), .color = center_color});
		// Second point of the triangle
		vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(center_position_px + full_span_px), .color = edge_color});
		// Rotate full_span_px for next iteration
		full_span_px = full_span_px.rotate(PI_MUL2 / static_cast<float>(steps));
		// Third point of the triangle
		vertices.push_back(SDL_Vertex{.position = static_cast<SDL_FPoint>(center_position_px + full_span_px), .color = edge_color});
	}
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	return SDL_RenderGeometry(renderer, nullptr, vertices.data(), (int)vertices.size(), nullptr, 0);
}

void m2::sdl::set_pixel(SDL_Surface* surface, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	set_pixel(surface, x, y, SDL_MapRGBA(surface->format, r, g, b, a));
}
void m2::sdl::set_pixel(SDL_Surface* surface, int x, int y, uint32_t pixel) {
	auto* target_pixel = (uint32_t*)((uint8_t*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
	*target_pixel = pixel;
}
std::optional<uint32_t> m2::sdl::get_pixel(const SDL_Surface* surface, int x, int y) {
	if (surface->format->format != SDL_PIXELFORMAT_BGRA32) {
		throw M2_ERROR("get_pixel is called with a surface with an unsupported pixel format");
	}
	if (x < 0 || y < 0) {
		return std::nullopt;
	}
	if (surface->w <= x || surface->h <= y) {
		return std::nullopt;
	}
	auto* target_pixel = (uint32_t*)((uint8_t*)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel);
	return *target_pixel;
}
