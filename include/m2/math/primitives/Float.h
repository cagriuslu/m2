#pragma once
#include <m2/M2.h>
#include <string>
#include <cfloat>

namespace m2 {
	class Float {
		float _value{};

	public:
		// Constructors

		Float() = default;
		explicit Float(const    int i) : _value(static_cast<float>(i)) {}
		explicit Float(const  float f) : _value(f) {}
		explicit Float(const double d) : _value(static_cast<float>(d)) {}

		// Operators

		explicit operator bool() const { return not IsZero(); }
		bool operator==(const Float& f) const { return _value == f._value; }

		// Attributes

		static Float Zero() { return Float{}; }
		static Float Max() { return Float{FLT_MAX}; }
		static Float Min() { return Float{-FLT_MAX}; }

		// Accessors

		[[nodiscard]] bool IsZero() const { return _value == 0.0f; }
		[[nodiscard]] bool IsPositive() const { return 0.0f < _value; }
		[[nodiscard]] bool IsNegative() const { return _value < 0.0f; }
		[[nodiscard]] int32_t ToInteger() const { return static_cast<int>(_value); }
		[[nodiscard]] float ToFloat() const { return _value; }
		[[nodiscard]] double ToDouble() const { return _value; }
		[[nodiscard]] std::string ToString() const;

		// Modifiers

		Float operator+() const { return *this; }
		Float operator-() const { return Float{-_value}; }
		Float operator+(const Float& b) const { return Float{_value + b._value}; }
		Float operator-(const Float& b) const { return Float{_value - b._value}; }
		Float operator*(const Float& b) const { return Float{_value * b._value}; }
		Float operator/(const Float& b) const { return Float{_value / b._value}; }
		[[nodiscard]] Float AbsoluteValue() const { return Float{fabsf(_value)}; }
	};
}
