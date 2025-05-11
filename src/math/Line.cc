#include <m2/math/Line.h>
#include <m2/Math.h>

float m2::Line::XIntersect() const {
	const auto slope = Slope();
	if (isinf(slope)) {
		return _point.x;
	}
	// 0 = (slope)x + yintersect
	return -YIntersect() / slope;
}
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

	float intersectionX, intersectionY;
	if (isinf(thisSlope)) {
		// This line is vertical
		intersectionX = this->_point.x;
		intersectionY = otherSlope * intersectionX + otherYIntersect;
	} else if (isinf(otherSlope)) {
		// Other line is vertical
		intersectionX = other._point.x;
		intersectionY = thisSlope * intersectionX + thisYIntersect;
	} else {
		intersectionX = (otherYIntersect - thisYIntersect) / (thisSlope - otherSlope);
		intersectionY = thisSlope * intersectionX + thisYIntersect;
	}
	return VecF{intersectionX, intersectionY};
}
float m2::Line::Angle() const {
	if (_parallel.x == 0.0f) {
		if (_parallel.y == 0.0f) {
			return NAN;
		}
		return PI_DIV2;
	}
	return atanf(_parallel.y / _parallel.x);
}
float m2::Line::AngleTo(const Line& other) const {
	const auto thisAngle = Angle();
	const auto otherAngle = other.Angle();
	const auto forwardSweepAngle = otherAngle - thisAngle;
	if (forwardSweepAngle < 0.0f) {
		return PI + forwardSweepAngle;
	}
	return forwardSweepAngle;
}
float m2::Line::SmallerAngleBetween(const Line& other) const {
	const auto angleToOther = AngleTo(other);
	if (PI_DIV2 < angleToOther) {
		return PI - angleToOther;
	}
	return angleToOther;
}
