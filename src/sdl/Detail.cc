#include <m2/sdl/Detail.h>
#include <m2/Log.h>
#include <m2/Game.h>
#include <m2/M2.h>
#include <cmath>

std::string m2::ToString(const SDL_Rect& rf) {
	std::stringstream ss;
	ss << "{x:" << rf.x << ",y:" << rf.y << ",w:" << rf.w << ",h:" << rf.h << "}";
	return ss.str();
}
std::string m2::ToString(const SDL_FRect& rf) {
	std::stringstream ss;
	ss << "{x:" << rf.x << ",y:" << rf.y << ",w:" << rf.w << ",h:" << rf.h << "}";
	return ss.str();
}

void m2::sdl::delay(ticks_t duration) {
	if (0 < duration) {
		SDL_Delay(U(duration));
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
