#include <m2/math/Line.h>

float m2::Line::YIntersect() const {
	// y = (slope)x + yintersect
	// y - (slope)x = yintersect
	return _point.y - Slope() * _point.x;
}

std::optional<m2::VecF> m2::Line::IntersectionWith(const Line& other) const {
	// Check if the slopes are very close
	if (_parallel.normalize().is_near(other._parallel.normalize(), 0.001f)) {
		return std::nullopt;
	}

	const auto thisSlope = Slope();
	const auto thisYIntersect = YIntersect();
	const auto otherSlope = other.Slope();
	const auto otherYIntersect = other.YIntersect();

	const auto intersectionX = (otherYIntersect - thisYIntersect) / (thisSlope - otherSlope);
	const auto intersectionY = thisSlope * intersectionX + thisYIntersect;
	return VecF{intersectionX, intersectionY};
}

float m2::Line::AngleBetween(const Line& other) const {
	const auto thisSlope = Slope();
	const auto otherSlope = other.Slope();
	return atanf(fabsf((thisSlope - otherSlope) / (1.0f + thisSlope * otherSlope)));
}
