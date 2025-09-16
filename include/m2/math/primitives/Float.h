#pragma once
#include <m2/M2.h>
#include <string>
#include <cfloat>

#include "m2/Math.h"

namespace m2 {
	class Float {
		float _value{};

	public:
		// Constructors

		Float() = default;
		explicit Float(const    int i) : _value(static_cast<float>(i)) {}
		explicit Float(const  float f) : _value(f) {}
		explicit Float(const double d) : _value(static_cast<float>(d)) {}
		static Float FromProtobufRepresentation(const double rawValueE6) { return Float{rawValueE6 / 1'000'000.0}; }

		// Attributes

		static Float Zero() { return Float{}; }
		static Float One() { return Float{1.0f}; }
		static Float SquareRootOf2() { return Float{SQROOT_2}; }
		static Float Max() { return Float{FLT_MAX}; }
		static Float Min() { return Float{-FLT_MAX}; }

		// Operators

		explicit operator bool() const { return not IsZero(); }
		friend bool operator==(const Float& a, const Float& b) { return a._value == b._value; }
		friend bool operator!=(const Float& a, const Float& b) { return a._value != b._value; }
		friend bool operator<(const Float& a, const Float& b) { return a._value < b._value; }
		friend bool operator<=(const Float& a, const Float& b) { return a._value <= b._value; }
		friend bool operator>(const Float& a, const Float& b) { return a._value > b._value; }
		friend bool operator>=(const Float& a, const Float& b) { return a._value >= b._value; }
		Float operator+() const { return *this; }
		Float operator-() const { return Float{-_value}; }
		Float operator+(const Float& b) const { return Float{_value + b._value}; }
		Float operator-(const Float& b) const { return Float{_value - b._value}; }
		Float operator*(const Float& b) const { return Float{_value * b._value}; }
		Float operator/(const Float& b) const { return Float{_value / b._value}; }
		Float& operator+=(const Float& other) { _value += other._value; return *this; }

		// Accessors

		[[nodiscard]] bool IsZero() const { return _value == 0.0f; }
		[[nodiscard]] bool IsPositive() const { return 0.0f < _value; }
		[[nodiscard]] bool IsNegative() const { return _value < 0.0f; }
		/// Check if the given number is equal enough to this number. Tolerance is assumed to be positive.
		[[nodiscard]] bool IsEqual(const Float& other, const Float& tolerance) const { return (*this - other).AbsoluteValue() <= tolerance; }
		[[nodiscard]] int32_t ToInteger() const { return static_cast<int>(_value); }
		[[nodiscard]] float ToFloat() const { return _value; }
		[[nodiscard]] double ToDouble() const { return _value; }
		[[nodiscard]] std::string ToString() const;

		// Modifiers

		[[nodiscard]] Float AbsoluteValue() const { return Float{fabsf(_value)}; }
		[[nodiscard]] Float Inverse() const { return Float{1.0f / _value}; }
		[[nodiscard]] Float Power(const Float& p) const { return Float{powf(_value, p._value)}; }
		[[nodiscard]] Float SquareRoot() const { return Float{sqrtf(_value)}; }
	};

	std::string ToString(const Float&);
}
