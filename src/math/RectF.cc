#include <m2/math/RectF.h>
#include <m2/M2.h>
#include <m2/math/RectI.h>
#include <m2/Meta.h>
#include <SDL2/SDL.h>
#include <sstream>
#include <m2/Math.h>

m2::RectF::RectF() : x(), y(), w(), h() {}
m2::RectF::RectF(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
m2::RectF::RectF(const VecF& xy, float w, float h) : x(xy.GetX()), y(xy.GetY()), w(w), h(h) {}
m2::RectF::RectF(const RectI& r) : x(static_cast<float>(r.x)), y(static_cast<float>(r.y)), w(static_cast<float>(r.w)), h(static_cast<float>(r.h)) {}
m2::RectF::RectF(const SDL_FRect& r) : x(r.x), y(r.y), w(r.w), h(r.h) {}
m2::RectF::RectF(const SDL_Rect& r) : x(static_cast<float>(r.x)), y(static_cast<float>(r.y)), w(static_cast<float>(r.w)), h(static_cast<float>(r.h)) {}
m2::RectF::RectF(const pb::RectI& r) : x(static_cast<float>(r.x())), y(static_cast<float>(r.y())), w(static_cast<float>(r.w())), h(static_cast<float>(r.h())) {}
m2::RectF m2::RectF::CreateFromCorners(const m2::VecF &corner1, const m2::VecF &corner2) {
	auto top_left_x = std::min(corner1.GetX(), corner2.GetX());
	auto top_left_y = std::min(corner1.GetY(), corner2.GetY());
	auto bottom_right_x = std::max(corner1.GetX(), corner2.GetX());
	auto bottom_right_y = std::max(corner1.GetY(), corner2.GetY());
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

float m2::RectF::GetArea() const {
	return w * h;
}

bool m2::RectF::IsEqual(const RectF& other, float tolerance) const {
	return m2::IsEqual(x, other.x, tolerance)
		&& m2::IsEqual(y, other.y, tolerance)
		&& m2::IsEqual(x + w, other.x + other.w, tolerance)
		&& m2::IsEqual(y + h, other.y + other.h, tolerance);
}

bool m2::RectF::DoesContain(const VecF& point, float tolerance) const {
	return IsLessOrEqual(x, point.GetX(), tolerance)
		&& IsLessOrEqual(point.GetX(), x + w, tolerance)
		&& IsLessOrEqual(y, point.GetY(), tolerance)
		&& IsLessOrEqual(point.GetY(), y + h, tolerance);
}

bool m2::RectF::DoesContain(const RectF& other, float tolerance) const {
	return IsLessOrEqual(x, other.x, tolerance)
		&& IsLessOrEqual(other.x + other.w, x + w, tolerance)
		&& IsLessOrEqual(y, other.y, tolerance)
		&& IsLessOrEqual(other.y + other.h, y + h, tolerance);
}

m2::RectF m2::RectF::Shift(const VecF& direction) const {
	return {x + direction.GetX(), y + direction.GetY(), w, h};
}
m2::RectF m2::RectF::ShiftCoordinateSystemOrigin(const VecF& direction) const {
	return {x - direction.GetX(), y - direction.GetY(), w, h};
}
m2::RectF m2::RectF::ExpandAllSides(float amount) const {
	return {x - amount, y - amount, w + amount + amount, h + amount + amount};
}
std::optional<m2::RectF> m2::RectF::GetIntersection(const RectF& other, float tolerance) const {
	if (not (*this) || not other) {
		return {};
	}

	// Stolen from SDL_IntersectRect
	float a_min, a_max, b_min, b_max;
	RectF result;

	// Horizontal intersection
	a_min = this->x;
	a_max = a_min + this->w;
	b_min = other.x;
	b_max = b_min + other.w;
	if (IsLessOrEqual(a_min, b_min, tolerance))
		a_min = b_min;
	result.x = a_min;
	if (IsLess(b_max, a_max, tolerance))
		a_max = b_max;
	result.w = a_max - a_min;

	// Vertical intersection
	a_min = this->y;
	a_max = a_min + this->h;
	b_min = other.y;
	b_max = b_min + other.h;
	if (IsLessOrEqual(a_min, b_min, tolerance))
		a_min = b_min;
	result.y = a_min;
	if (IsLess(b_max, a_max, tolerance))
		a_max = b_max;
	result.h = a_max - a_min;

	return result ? result : std::optional<m2::RectF>{};
}
std::vector<m2::VecI> m2::RectF::GetIntersectingCells() const {
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
m2::VecF m2::RectF::GetCenterPoint() const {
	return VecF{x + w / 2.0f, y + h / 2.0f};
}

std::string m2::ToString(const RectF& rect) {
	std::stringstream ss;
	ss << "{x:" << rect.x << ",y:" << rect.y << ",w:" << rect.w << ",h:" << rect.h << "}";
	return ss.str();
}
