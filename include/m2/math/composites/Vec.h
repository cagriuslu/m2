#pragma once

namespace m2 {
	template <typename Primitive>
	class Vec {
		Primitive _x, _y;

	public:
		// Constructors

		Vec() = default;
		Vec(const Primitive& x, const Primitive& y) : _x(x), _y(y) {}
		Vec(Primitive&& x, Primitive&& y) : _x(std::move(x)), _y(std::move(y)) {}

		// Operators

		explicit operator bool() const { return static_cast<bool>(_x) || static_cast<bool>(_y); }
		bool operator==(const Vec& other) const { return _x == other._x && _y == other._y; }

		// Accessors

		[[nodiscard]] bool IsNear(const Vec& other, Primitive tolerance) const { return (other._x - _x).AbsoluteValue() <= tolerance && (other._y - _y).AbsoluteValue() <= tolerance; }
		[[nodiscard]] Primitive Magnitude() const { /* TODO */ }
		[[nodiscard]] Primitive MagnitudeSquare() const { /* TODO */ }
	};
}
