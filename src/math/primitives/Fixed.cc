#include <m2/math/primitives/Fixed.h>
#include <cstdio>
#include <array>
#include <ranges>
#include <algorithm>

static int PrecisionPointToDecimalLut[] = {
	50000000, // 1 / (2^1)
	25000000, // 1 / (2^2)
	12500000, // 1 / (2^3)
	 6250000, // 1 / (2^4)
	 3125000, // 1 / (2^5)
	 1562500, // 1 / (2^6)
	  781250, // 1 / (2^7)
	  390625, // 1 / (2^8)
	  195312, // 1 / (2^9)
	   97656, // 1 / (2^10)
	   48828, // 1 / (2^11)
	   24414, // 1 / (2^12)
	   12207, // 1 / (2^13)
		6104, // 1 / (2^14)
		3052, // 1 / (2^15)
		1526, // 1 / (2^16)
		 763, // 1 / (2^17)
		 381, // 1 / (2^18)
		 191, // 1 / (2^19)
		  95, // 1 / (2^20)
		  48, // 1 / (2^21)
		  24, // 1 / (2^22)
	      12, // 1 / (2^23)
	       6, // 1 / (2^24)
	       3, // 1 / (2^25)
};

std::string m2::Fixed::ToString() const {
	if constexpr (std::size(PrecisionPointToDecimalLut) < PRECISION) {
		throw M2_ERROR("Implementation error, precision not supported");
	}

	std::array<char, 32> buffer{};
	std::ranges::fill(buffer, 0);

	int seek = 0;
	if (IsNegative()) {
		buffer[seek++] = '-';
	} else {
		buffer[seek++] = '+';
	}

	// First, print the integer part
	auto integer_part =
		(_value == static_cast<int>(0x80000000))
		? (1 << (SIGNIFICANT - 1)) // Special case where the negative number doesn't fit into SIGNIFICANT number of bits
		: IsNegative() ? Fixed{std::in_place, -_value}.ToInteger() : ToInteger();
	seek += snprintf(buffer.data() + seek, buffer.size() - seek, "%06d", integer_part);

	// Then, print the radix point
	buffer[seek++] = '.';

	// Then, print the fractional part
	// Find the absolute value of the fractional part
	auto fractional_part = (IsNegative() ? -_value : _value) & FRACTION_PART_MASK;
	// Iterate from the most significant fractional bit to the least significant
	auto fraction = 0;
	for (int i = 0; i < PRECISION; ++i) {
		auto bit = fractional_part & (1 << (PRECISION - i - 1));
		if (bit) {
			fraction += PrecisionPointToDecimalLut[i];
		}
	}
	// Append the fraction to the buffer
	snprintf(buffer.data() + seek, buffer.size() - seek, "%08d", fraction);

	return {buffer.data()};
}
std::string m2::Fixed::ToFastString() const {
	std::array<char, 32> buffer{};
	std::ranges::fill(buffer, 0);
	snprintf(buffer.data(), buffer.size(), "%+016.8f", ToDouble());
	return {buffer.data()};
}
std::string m2::Fixed::ToFastestString() const {
	std::array<char, 32> buffer{};
	std::ranges::fill(buffer, 0);
	snprintf(buffer.data(), buffer.size(), "%+016.08f", ToFloat());
	return {buffer.data()};
}

void m2::Fixed::ThrowIfOutOfBounds(const int i) {
	if (Max().ToInteger() < i) {
		throw M2_ERROR("Integer is more than what Fixed can hold");
	}
	if (i < Min().ToInteger()) {
		throw M2_ERROR("Integer is less than what Fixed can hold");
	}
}
void m2::Fixed::ThrowIfOutOfBounds(const float f) {
	if (Max().ToFloat() < f) {
		throw M2_ERROR("Float is more than what Fixed can hold");
	}
	if (f < Min().ToFloat()) {
		throw M2_ERROR("Float is less than what Fixed can hold");
	}
}
void m2::Fixed::ThrowIfOutOfBounds(const double d) {
	if (Max().ToDouble() < d) {
		throw M2_ERROR("Double is more than what Fixed can hold");
	}
	if (d < Min().ToDouble()) {
		throw M2_ERROR("Double is less than what Fixed can hold");
	}
}

std::string m2::ToString(const Fixed& f) {
	return f.ToString();
}
