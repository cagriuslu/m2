#pragma once
#include <m2/M2.h>
#include <string>

namespace m2 {
	class Exact final {
		// PRECISION=14 divides [0, 1) into 2^14 (16384) pieces. This guarantees 4 digits of decimal fractions.
		static constexpr auto PRECISION = 14;
		// SIGNIFICANT=18 extends to [-131072, 131071).
		static constexpr auto SIGNIFICANT = 32 - PRECISION;

		static constexpr int32_t INTEGER_PART_MASK = I(0xFFFFFFFF << PRECISION);
		static constexpr int32_t FRACTION_PART_MASK = I(0xFFFFFFFFu >> SIGNIFICANT);
		static constexpr int32_t SIGN_BIT_MASK = I(0x80000000);
		static constexpr int32_t LEAST_SIGNIFICANT_INTEGER_BIT_MASK = I(1 << PRECISION);
		static constexpr int32_t MOST_SIGNIFICANT_FRACTION_BIT_MASK = I(1 << (PRECISION - 1));

		int32_t _value{};

		static Exact UnsafeFromInt(const int i)       noexcept { return Exact{std::in_place, i << PRECISION}; }
		static Exact UnsafeFromFloat(const float f)   noexcept { return Exact{std::in_place, RoundI(::m2::ToFloat(LEAST_SIGNIFICANT_INTEGER_BIT_MASK) * f)}; }
		static Exact UnsafeFromDouble(const double d) noexcept { return Exact{std::in_place, RoundI(D(LEAST_SIGNIFICANT_INTEGER_BIT_MASK) * d)}; }

	public:
		// Constructors

		Exact() = default;
		explicit Exact(std::in_place_t, const int32_t value) noexcept : _value(value) {}
		explicit Exact(const    int i) { ThrowIfOutOfBounds(i); *this = UnsafeFromInt(i);    }
		explicit Exact(const  float f) { ThrowIfOutOfBounds(f); *this = UnsafeFromFloat(f);  }
		explicit Exact(const double d) { ThrowIfOutOfBounds(d); *this = UnsafeFromDouble(d); }

		// Attributes

		static Exact Zero() { return Exact{}; }
		static Exact One() { return Exact{1}; }
		static Exact SquareRootOf2() { return Exact{std::in_place, 0b00000000'00000000'01011010'10000010}; }
		static Exact Max() { return Exact{std::in_place, 0x7FFFFFFF}; }
		static Exact Min() { return Exact{std::in_place, static_cast<int32_t>(0x80000000)}; }
		static Exact MaxInteger() { return Exact{std::in_place, (0xFFFFFFFFu << PRECISION) & 0x7FFFFFFF}; }
		static Exact MinInteger() { return Min(); }
		static expected<Exact> Compose(int32_t wholePart, int32_t fractionalPart);
		/// Returns the closest Exact number to the given decimal number. This operation is deterministic.
		static expected<Exact> ClosestExact(std::string_view);

		static constexpr int32_t RAW_1DIV2 = 1 << (PRECISION - 1);
		static constexpr int32_t RAW_1DIV4 = 1 << (PRECISION - 2);
		static constexpr int32_t RAW_1DIV8 = 1 << (PRECISION - 3);
		static constexpr int32_t RAW_1DIV16 = 1 << (PRECISION - 4);
		static constexpr int32_t RAW_1DIV32 = 1 << (PRECISION - 5);
		static constexpr int32_t RAW_1DIV64 = 1 << (PRECISION - 6);
		static constexpr int32_t RAW_1DIV128 = 1 << (PRECISION - 7);
		static constexpr int32_t RAW_1DIV256 = 1 << (PRECISION - 8);
		static constexpr int32_t RAW_1DIV512 = 1 << (PRECISION - 9);
		static constexpr int32_t RAW_1DIV1024 = 1 << (PRECISION - 10);

		static constexpr int32_t RAW_0P5 = RAW_1DIV2;
		static constexpr int32_t RAW_0P25 = RAW_1DIV4;
		static constexpr int32_t RAW_0P125 = RAW_1DIV8;
		static constexpr int32_t RAW_0P0625 = RAW_1DIV16;
		static constexpr int32_t RAW_0P03125 = RAW_1DIV32;
		static constexpr int32_t RAW_0P015625 = RAW_1DIV64;
		static constexpr int32_t RAW_0P0078125 = RAW_1DIV128;
		static constexpr int32_t RAW_0P00390625 = RAW_1DIV256;
		static constexpr int32_t RAW_0P001953125 = RAW_1DIV512;
		static constexpr int32_t RAW_0P0009765625 = RAW_1DIV1024;

		// Operators

		explicit operator bool() const { return not IsZero(); }
		friend bool operator==(const Exact& a, const Exact& b) { return a._value == b._value; }
		friend bool operator!=(const Exact& a, const Exact& b) { return a._value != b._value; }
		friend bool operator<(const Exact& a, const Exact& b) { return a._value < b._value; }
		friend bool operator<=(const Exact& a, const Exact& b) { return a._value <= b._value; }
		friend bool operator>(const Exact& a, const Exact& b) { return a._value > b._value; }
		friend bool operator>=(const Exact& a, const Exact& b) { return a._value >= b._value; }
		Exact operator+() const { return *this; }
		Exact operator-() const { return Exact{std::in_place, -_value}; }
		Exact operator+(const Exact& b) const { return Exact{std::in_place, _value + b._value}; }
		Exact operator-(const Exact& b) const { return Exact{std::in_place, _value - b._value}; }
		Exact operator*(const Exact& b) const { return Exact{std::in_place, static_cast<int32_t>((static_cast<int64_t>(_value) * static_cast<int64_t>(b._value)) >> PRECISION)}; }
		Exact operator/(const Exact& b) const { return Exact{std::in_place, static_cast<int32_t>((static_cast<int64_t>(_value) << PRECISION) / static_cast<int64_t>(b._value))}; }
		Exact& operator+=(const Exact& other) { _value += other._value; return *this; }

		// Accessors

		[[nodiscard]] bool IsZero() const { return _value == 0; }
		[[nodiscard]] bool IsPositive() const { return not IsZero() && not IsNegative(); }
		[[nodiscard]] bool IsNegative() const { return _value & 0x80000000; }
		/// Tolerance is assumed to be positive. Provided for API compatible with Float.
		[[nodiscard]] bool IsEqual(const Exact& other, const Exact& tolerance) const { return (*this - other).AbsoluteValue() <= tolerance; }
		/// Tolerance is assumed to be positive. Provided for API compatible with Float.
		[[nodiscard]] bool IsNotEqual(const Exact& other, const Exact& tolerance) const { return tolerance < (*this - other).AbsoluteValue(); }
		/// Tolerance is assumed to be positive. Provided for API compatible with Float.
		[[nodiscard]] bool IsLess(const Exact& other, const Exact& tolerance) const { return tolerance < (other - *this); }
		/// Tolerance is assumed to be positive. Provided for API compatible with Float.
		[[nodiscard]] bool IsLessOrEqual(const Exact& other, const Exact& tolerance) const { return IsLess(other, tolerance) || IsEqual(other, tolerance); }
		/// The result is an approximation because the fractional part of the number will be thrown away
		[[nodiscard]] int32_t ToInteger() const { return _value >> PRECISION; }
		/// The result is an approximation because floating point numbers get less accurate away from origin
		[[nodiscard]] float ToFloat() const { return ::m2::ToFloat(_value) / ::m2::ToFloat(0x1 << PRECISION); }
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

		[[nodiscard]] Exact AbsoluteValue() const { return IsNegative() ? -*this : *this; }
		[[nodiscard]] Exact Inverse() const { return One() / *this; }
		[[nodiscard]] Exact SquareRoot() const;
		[[nodiscard]] Exact Round() const; /// Round to nearest integer. Halfway values are rounded away from zero.

	private:
		static void ThrowIfOutOfBounds(int i);
		static void ThrowIfOutOfBounds(float f);
		static void ThrowIfOutOfBounds(double d);

		friend int32_t ToRawValue(const Exact&);
	};

	std::string ToString(const Exact&);
	inline int32_t ToRawValue(const Exact& e) { return e._value; }
	/// Provided for API compatibility. Throws when called.
	inline int32_t ToRawValue(const float) { throw M2_ERROR("Forbidden raw value conversion on float"); }
}

constexpr m2::expected<m2::Exact> operator ""_closest_exact(const char* str, const std::size_t sz) {
	return m2::Exact::ClosestExact(std::string_view{str, sz});
}
