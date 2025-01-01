#pragma once
#include <utility>
#include <sstream>

namespace m2 {
	template <typename Primitive>
	class Vec {
		Primitive _x, _y;

	public:
		// Constructors

		Vec() = default;
		Vec(const Primitive& x, const Primitive& y) : _x(x), _y(y) {}
		Vec(Primitive&& x, Primitive&& y) : _x(std::move(x)), _y(std::move(y)) {}
		Vec(int x, int y) : _x(x), _y(y) {}
		Vec(float x, float y) : _x(x), _y(y) {}

		// Operators

		explicit operator bool() const { return static_cast<bool>(_x) || static_cast<bool>(_y); }
		bool operator==(const Vec& other) const { return _x == other._x && _y == other._y; }
		Vec operator+(const Vec& other) const { return {_x + other._x, _y + other._y}; }
		Vec operator-(const Vec& other) const { return {_x - other._x, _y - other._y}; }
		Vec& operator+=(const Vec& other) { _x += other._x; _y += other._y; return *this; }
		Vec operator*(const Primitive& p) const { return {_x * p, _y * p}; }
		Vec operator/(const Primitive& p) const { return {_x / p, _y / p}; }

		// Accessors

		[[nodiscard]] const Primitive& X() const { return _x; }
		[[nodiscard]] const Primitive& Y() const { return _y; }
		[[nodiscard]] bool IsNear(const Vec& other, Primitive tolerance) const { return (other._x - _x).AbsoluteValue() <= tolerance && (other._y - _y).AbsoluteValue() <= tolerance; }
		[[nodiscard]] Primitive Magnitude() const { return MagnitudeSquare().SquareRoot(); }
		[[nodiscard]] Primitive MagnitudeSquare() const { return _x * _x + _y * _y; }
	};

	template <typename Primitive>
	std::string ToString(const Vec<Primitive>& v) {
		std::stringstream ss;
		ss << "{x:" << ToString(v.X()) << ",y:" << ToString(v.Y()) << "}";
		return ss.str();
	}
}
