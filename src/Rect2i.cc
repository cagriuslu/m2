#include <m2/Rect2i.h>
#include <m2/Rect2f.h>
#include <sstream>

m2::Rect2i::Rect2i() : x(), y(), w(), h() {}
m2::Rect2i::Rect2i(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}
m2::Rect2i::Rect2i(const Rect2f& r) : x(static_cast<int>(r.x)), y(static_cast<int>(r.y)), w(static_cast<int>(r.w)), h(static_cast<int>(r.h)) {}
m2::Rect2i::Rect2i(const SDL_Rect& r) : x(r.x), y(r.y), w(r.w), h(r.h) {}
m2::Rect2i::Rect2i(const pb::Rect2i& r) : x(r.x()), y(r.y()), w(r.w()), h(r.h()) {}

bool m2::Rect2i::operator==(const Rect2i &other) const {
	return (x == other.x) && (y == other.y) && (w == other.w) && (h == other.h);
}
m2::Rect2i::operator bool() const {
	return 0 < w && 0 < h;
}
m2::Rect2i::operator SDL_Rect() const {
	return SDL_Rect{x, y, w, h};
}
m2::Rect2i::operator SDL_FRect() const {
	return SDL_FRect{static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)};
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
m2::Rect2i m2::Rect2i::trim_to_square() const {
	if (h < w) {
		auto diff = w - h;
		return trim_left(diff / 2).trim_right(diff / 2);
	} else if (w < h) {
		auto diff = h - w;
		return trim_top(diff / 2).trim_bottom(diff / 2);
	}
	return *this;
}
m2::Rect2i m2::Rect2i::expand(int amount) const {
	return trim(-amount);
}
std::optional<m2::Rect2i> m2::Rect2i::intersect(const m2::Rect2i& other) const {
	auto a = static_cast<SDL_Rect>(*this);
	auto b = static_cast<SDL_Rect>(other);
	SDL_Rect result;
	if (SDL_IntersectRect(&a, &b, &result)) {
		return Rect2i{result};
	} else {
		return {};
	}
}

std::string m2::to_string(const m2::Rect2i& v) {
	std::stringstream ss;
	ss << "{x:" << v.x << ",y:" << v.y << ",w:" << v.w << ",h:" << v.h << "}";
	return ss.str();
}
