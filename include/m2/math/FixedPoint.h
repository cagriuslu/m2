#pragma once
#include "../M2.h"
#include <string>
#include <cstdint>

namespace m2 {
	class FixedPoint final {
		// PRECISION=14 divides [0, 1) into 2^14 (16384) pieces. This guarantees 4 digits of decimal fractions.
		static constexpr int PRECISION = 14;
		// SIGNIFICANT=18 extends to [-131072, 131071).
		static constexpr int SIGNIFICANT = 32 - PRECISION;

		int32_t _value{};

	public:
		// Constructors

		FixedPoint() = default;
		explicit FixedPoint(std::in_place_t, const int32_t value) noexcept : _value(value) {}
		explicit FixedPoint(const    int i) { ThrowIfOutOfBounds(i); *this = UnsafeFromInt(i);    }
		explicit FixedPoint(const  float f) { ThrowIfOutOfBounds(f); *this = UnsafeFromFloat(f);  }
		explicit FixedPoint(const double d) { ThrowIfOutOfBounds(d); *this = UnsafeFromDouble(d); }
		static FixedPoint UnsafeFromInt(int i)       noexcept { return FixedPoint{std::in_place, i << PRECISION}; }
		static FixedPoint UnsafeFromFloat(float f)   noexcept { return FixedPoint{std::in_place, iround(F(0x1 << PRECISION) * f)}; }
		static FixedPoint UnsafeFromDouble(double d) noexcept { return FixedPoint{std::in_place, iround(D(0x1 << PRECISION) * d)}; }

		// Operators

		explicit operator bool() const { return not IsZero(); }
		bool operator==(const FixedPoint& b) const { return _value == b._value; }

		// Attributes

		static FixedPoint Zero() { return FixedPoint{}; }
		static FixedPoint Max() { return FixedPoint{std::in_place, 0x7FFFFFFF}; }
		static FixedPoint Min() { return FixedPoint{std::in_place, static_cast<int32_t>(0x80000000)}; }
		static FixedPoint MaxInteger() { return FixedPoint{std::in_place, (0xFFFFFFFFu << PRECISION) & 0x7FFFFFFF}; }
		static FixedPoint MinInteger() { return Min(); }
		static uint32_t IntegerPartMask()    { return 0xFFFFFFFFu << PRECISION; }
		static uint32_t FractionalPartMask() { return 0xFFFFFFFFu >> SIGNIFICANT; }

		// Accessors

		[[nodiscard]] bool IsZero() const { return _value == 0; }
		[[nodiscard]] bool IsPositive() const { return not IsZero() && not IsNegative(); }
		[[nodiscard]] bool IsNegative() const { return _value & 0x80000000; }
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

		FixedPoint operator+() const { return *this; }
		FixedPoint operator-() const { return FixedPoint{std::in_place, -_value}; }
		FixedPoint operator+(const FixedPoint& b) const { return FixedPoint{std::in_place, _value + b._value}; }
		FixedPoint operator-(const FixedPoint& b) const { return FixedPoint{std::in_place, _value - b._value}; }
		FixedPoint operator*(const FixedPoint& b) const { return FixedPoint{std::in_place, static_cast<int32_t>((static_cast<int64_t>(_value) * static_cast<int64_t>(b._value)) >> PRECISION)}; }
		FixedPoint operator/(const FixedPoint& b) const { return FixedPoint{std::in_place, static_cast<int32_t>((static_cast<int64_t>(_value) << PRECISION) / static_cast<int64_t>(b._value))}; }

	private:
		static void ThrowIfOutOfBounds(int i);
		static void ThrowIfOutOfBounds(float f);
		static void ThrowIfOutOfBounds(double d);
	};
}
