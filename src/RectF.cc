#include <m2/RectF.h>
#include <m2/RectI.h>
#include <m2/Meta.h>
#include <sstream>

m2::RectF::RectF() : x(), y(), w(), h() {}
m2::RectF::RectF(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
m2::RectF::RectF(const VecF& offset_from_origin, const VecF& dims) : x(offset_from_origin.x - dims.x / 2.0f), y(offset_from_origin.y - dims.y / 2.0f), w(dims.x), h(dims.y) {}
m2::RectF::RectF(const RectI& r) : x(static_cast<float>(r.x)), y(static_cast<float>(r.y)), w(static_cast<float>(r.w)), h(static_cast<float>(r.h)) {}
m2::RectF::RectF(const SDL_FRect& r) : x(r.x), y(r.y), w(r.w), h(r.h) {}
m2::RectF::RectF(const SDL_Rect& r) : x(static_cast<float>(r.x)), y(static_cast<float>(r.y)), w(static_cast<float>(r.w)), h(static_cast<float>(r.h)) {}
m2::RectF::RectF(const pb::RectI& r) : x(static_cast<float>(r.x())), y(static_cast<float>(r.y())), w(static_cast<float>(r.w())), h(static_cast<float>(r.h())) {}
m2::RectF m2::RectF::from_corners(const m2::VecF &corner1, const m2::VecF &corner2) {
	auto top_left_x = std::min(corner1.x, corner2.x);
	auto top_left_y = std::min(corner1.y, corner2.y);
	auto bottom_right_x = std::max(corner1.x, corner2.x);
	auto bottom_right_y = std::max(corner1.y, corner2.y);
	return {top_left_x, top_left_y, bottom_right_x - top_left_x, bottom_right_y - top_left_y};
}

m2::RectF::operator bool() const {
	return 0.0f < w && 0.0f < h;
}
m2::RectF::operator SDL_FRect() const {
	return SDL_FRect{x, y, w, h};
}
m2::RectF::operator SDL_Rect() const {
	return SDL_Rect{static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h)};
}

float m2::RectF::area() const {
	return w * h;
}
m2::RectF m2::RectF::shift(const VecF& direction) const {
	return {x + direction.x, y + direction.y, w, h};
}
m2::RectF m2::RectF::shift_origin(const VecF& direction) const {
	return {x - direction.x, y - direction.y, w, h};
}
m2::RectF m2::RectF::expand(float amount) const {
	return {x - amount, y - amount, w + amount + amount, h + amount + amount};
}
std::optional<m2::RectF> m2::RectF::intersect(const RectF& other) const {
	if (not (*this) || not other) {
		return {};
	}

	// Stolen from SDL_IntersectRect
	float Amin, Amax, Bmin, Bmax;
	RectF result;

	// Horizontal intersection
	Amin = this->x;
	Amax = Amin + this->w;
	Bmin = other.x;
	Bmax = Bmin + other.w;
	if (Bmin > Amin)
		Amin = Bmin;
	result.x = Amin;
	if (Bmax < Amax)
		Amax = Bmax;
	result.w = Amax - Amin;

	// Vertical intersection
	Amin = this->y;
	Amax = Amin + this->h;
	Bmin = other.y;
	Bmax = Bmin + other.h;
	if (Bmin > Amin)
		Amin = Bmin;
	result.y = Amin;
	if (Bmax < Amax)
		Amax = Bmax;
	result.h = Amax - Amin;

	return result ? result : std::optional<m2::RectF>{};
}
std::vector<m2::VecI> m2::RectF::intersecting_cells() const {
	std::vector<m2::VecI> cells;

	int lower_x = I(floorf(x)), higher_x = I(ceilf(x + w));
	int lower_y = I(floorf(y)), higher_y = I(ceilf(y + h));
	for (int b = lower_y; b < higher_y; ++b) {
		for (int a = lower_x; a < higher_x; ++a) {
			cells.emplace_back(a, b);
		}
	}
	return cells;
}
m2::VecF m2::RectF::center() const {
	return VecF{x + w / 2.0f, y + h / 2.0f};
}

std::string m2::to_string(const RectF& rect) {
	std::stringstream ss;
	ss << "{x:" << rect.x << ",y:" << rect.y << ",w:" << rect.w << ",h:" << rect.h << "}";
	return ss.str();
}
