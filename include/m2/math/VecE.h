#pragma once
#include <m2/math/primitives/Exact.h>
#include <m2/math/primitives/Float.h>
#include <m2/ProxyTypes.h>

namespace m2 {
	class VecF;

	class VecE {
		Exact _x, _y;

	public:
		// Constructors

		VecE() = default;
		VecE(const Exact& x, const Exact& y) : _x(x), _y(y) {}
		VecE(const int x, const int y) : _x(x), _y(y) {}
		VecE(Exact&& x, Exact&& y) : _x(std::move(x)), _y(std::move(y)) {}
		explicit VecE(const VecF&);

		// Operators

		VecE operator+(const VecE& rhs) const { return {_x + rhs._x, _y + rhs._y}; }
		VecE& operator+=(const VecE& rhs) { _x += rhs._x; _y += rhs._y; return *this; }
		VecE& operator-=(const VecE& rhs) { _x -= rhs._x; _y -= rhs._y; return *this; }
		VecE operator-() const { return {-_x, -_y}; }
		VecE operator-(const VecE& rhs) const { return {_x - rhs._x, _y - rhs._y}; }
		VecE operator-(const Exact& rhs) const { return {_x - rhs, _y - rhs}; }
		VecE operator*(const Exact& rhs) const { return {_x * rhs, _y * rhs}; }
		VecE operator/(const Exact& rhs) const { return {_x / rhs, _y / rhs}; }
		explicit operator VecF() const;

		// Accessors

		[[nodiscard]] const Exact& GetX() const { return _x; }
		[[nodiscard]] const Exact& GetY() const { return _y; }
		[[nodiscard]] Exact GetLengthSquaredFE() const;
		[[nodiscard]] Exact GetLengthFE() const { return GetLengthSquaredFE().SquareRoot(); }
		[[nodiscard]] Exact GetDistanceToSquaredFE(const VecE& other) const;
		[[nodiscard]] Exact GetDistanceToFE(const VecE& other) const { return (other - *this).GetLengthFE(); }

		// Modifiers

		[[nodiscard]] VecE Normalize() const { const auto len = GetLengthFE(); return len ? VecE{_x / len, _y / len} : VecE{}; }
	};
}
