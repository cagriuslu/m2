#pragma once
#include <m2/M2.h>
#include <string>
#include <cstdint>

namespace m2 {
	class Fixed final {
		// PRECISION=14 divides [0, 1) into 2^14 (16384) pieces. This guarantees 4 digits of decimal fractions.
		static constexpr int PRECISION = 14;
		// SIGNIFICANT=18 extends to [-131072, 131071).
		static constexpr int SIGNIFICANT = 32 - PRECISION;

		int32_t _value{};

	public:
		// Constructors

		Fixed() = default;
		explicit Fixed(std::in_place_t, const int32_t value) noexcept : _value(value) {}
		explicit Fixed(const    int i) { ThrowIfOutOfBounds(i); *this = UnsafeFromInt(i);    }
		explicit Fixed(const  float f) { ThrowIfOutOfBounds(f); *this = UnsafeFromFloat(f);  }
		explicit Fixed(const double d) { ThrowIfOutOfBounds(d); *this = UnsafeFromDouble(d); }
		static Fixed UnsafeFromInt(int i)       noexcept { return Fixed{std::in_place, i << PRECISION}; }
		static Fixed UnsafeFromFloat(float f)   noexcept { return Fixed{std::in_place, iround(F(0x1 << PRECISION) * f)}; }
		static Fixed UnsafeFromDouble(double d) noexcept { return Fixed{std::in_place, iround(D(0x1 << PRECISION) * d)}; }

		// Operators

		explicit operator bool() const { return not IsZero(); }
		bool operator==(const Fixed& b) const { return _value == b._value; }
		bool operator<=(const Fixed& b) const { return _value <= b._value; }
		Fixed operator+() const { return *this; }
		Fixed operator-() const { return Fixed{std::in_place, -_value}; }
		Fixed operator+(const Fixed& b) const { return Fixed{std::in_place, _value + b._value}; }
		Fixed operator-(const Fixed& b) const { return Fixed{std::in_place, _value - b._value}; }
		Fixed operator*(const Fixed& b) const { return Fixed{std::in_place, static_cast<int32_t>((static_cast<int64_t>(_value) * static_cast<int64_t>(b._value)) >> PRECISION)}; }
		Fixed operator/(const Fixed& b) const { return Fixed{std::in_place, static_cast<int32_t>((static_cast<int64_t>(_value) << PRECISION) / static_cast<int64_t>(b._value))}; }
		friend bool operator<(const Fixed& a, const Fixed& b) { return a._value < b._value; }

		// Attributes

		static Fixed Zero() { return Fixed{}; }
		static Fixed Max() { return Fixed{std::in_place, 0x7FFFFFFF}; }
		static Fixed Min() { return Fixed{std::in_place, static_cast<int32_t>(0x80000000)}; }
		static Fixed MaxInteger() { return Fixed{std::in_place, (0xFFFFFFFFu << PRECISION) & 0x7FFFFFFF}; }
		static Fixed MinInteger() { return Min(); }
		static uint32_t IntegerPartMask()    { return 0xFFFFFFFFu << PRECISION; }
		static uint32_t FractionalPartMask() { return 0xFFFFFFFFu >> SIGNIFICANT; }

		// Accessors

		[[nodiscard]] bool IsZero() const { return _value == 0; }
		[[nodiscard]] bool IsPositive() const { return not IsZero() && not IsNegative(); }
		[[nodiscard]] bool IsNegative() const { return _value & 0x80000000; }
		// Check if the given number is equal enough to this number. Tolerance is assumed to be positive.
		// Fixed point numbers should not have an equality conversion problem, but this functions is provided to be API
		// compatible with Float.
		[[nodiscard]] bool IsEqual(const Fixed& other, const Fixed& tolerance) const { return (*this - other).AbsoluteValue() <= tolerance; }
		/// The result is an approximation because the fractional part of the number will be thrown away
		[[nodiscard]] int32_t ToInteger() const { return _value >> PRECISION; }
		/// The result is an approximation because floating point numbers get less accurate away from origin
		[[nodiscard]] float ToFloat() const { return F(_value) / F(0x1 << PRECISION); }
		/// The result is an approximation because floating point numbers get less accurate away from origin
		[[nodiscard]] double ToDouble() const { return D(_value) / D(0x1 << PRECISION); }
		/// Returns the raw value contained inside
		[[nodiscard]] int32_t ToRawValue() const { return _value; }
		/// The result is an approximation because only 8 digits of fraction is printed.
		[[nodiscard]] std::string ToString() const;
		/// The result is an approximation because ToDouble() is used internally, and it's less accurate than ToString().
		[[nodiscard]] std::string ToFastString() const;
		/// The result is an approximation because ToFloat() is used internally, and it's less accurate than ToFastString().
		[[nodiscard]] std::string ToFastestString() const;

		// Modifiers

		[[nodiscard]] Fixed AbsoluteValue() const { return IsNegative() ? -*this : *this; }
		[[nodiscard]] Fixed Inverse() const { return Fixed{1} / *this; }

	private:
		static void ThrowIfOutOfBounds(int i);
		static void ThrowIfOutOfBounds(float f);
		static void ThrowIfOutOfBounds(double d);
	};

	std::string to_string(const Fixed&);
}
