#include <m2/Rect2i.h>
#include <sstream>

m2::Rect2i::Rect2i() : x(), y(), w(), h() {}
m2::Rect2i::Rect2i(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
m2::Rect2i::Rect2i(const SDL_Rect& r) : x(r.x), y(r.y), w(r.w), h(r.h) {}

bool m2::Rect2i::operator==(const Rect2i &other) const {
	return (x == other.x) && (y == other.y) && (w == other.w) && (h == other.h);
}
m2::Rect2i::operator bool() const {
	return 0 < w && 0 < h;
}
m2::Rect2i::operator SDL_Rect() const {
	return SDL_Rect{x, y, w, h};
}
bool m2::Rect2i::point_in_rect(const Vec2i& p) const {
	return (p.x >= x) && (p.x < (x + w)) && (p.y >= y) && (p.y < (y + h));
}

m2::Rect2i m2::Rect2i::trim(int amount) const {
	// TODO bound check
	return Rect2i{x + amount, y + amount, w - amount - amount, h - amount - amount};
}
m2::Rect2i m2::Rect2i::trim_left(int amount) const {
	return (amount < w) ? Rect2i{x + amount, y, w - amount, h} : Rect2i{x + w, y, 0, h};
}
m2::Rect2i m2::Rect2i::trim_right(int amount) const {
	return (amount < w) ? Rect2i{x, y, w - amount, h} : Rect2i{x, y, 0, h};
}
m2::Rect2i m2::Rect2i::trim_top(int amount) const {
	return (amount < h) ? Rect2i{x, y + amount, w, h - amount} : Rect2i{x, y + h, w, 0};
}
m2::Rect2i m2::Rect2i::trim_bottom(int amount) const {
	return (amount < h) ? Rect2i{x, y, w, h - amount} : Rect2i{x, y, w, 0};
}
m2::Rect2i m2::Rect2i::expand(int amount) const {
	return trim(-amount);
}

std::string m2::to_string(const m2::Rect2i& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << ",w:" << v.w << ",h:" << v.h << "}";
	return ss.str();
}
