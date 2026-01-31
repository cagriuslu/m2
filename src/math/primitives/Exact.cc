#include <m2/math/primitives/Exact.h>
#include <cstdio>
#include <array>
#include <ranges>
#include <algorithm>

namespace {
	// Look up table where i'th element corresponds to the value of 1/(2^i) multiplied by 10^8.
	// This is used while converting a fixed point number to string.
	const int PRECISION_POINT_TO_DECIMAL_8[] = {
		50000000, // 1 / (2^1)
		25000000, // 1 / (2^2)
		12500000, // 1 / (2^3)
		 6250000, // 1 / (2^4)
		 3125000, // 1 / (2^5)
		 1562500, // 1 / (2^6)
		  781250, // 1 / (2^7)
		  390625, // 1 / (2^8)
		  195313, // 1 / (2^9)
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
			   1, // 1 / (2^26)
	};

	int FindMostSignificantSetBit(const uint64_t value) {
		if (value == 0) {
			throw M2_ERROR("Attempt to find the most significant bit of zero");
		}
		uint64_t search = 0x8000000000000000llu;
		for (int i = 0; i < 64; ++i) {
			if (value & search) {
				return 64 - i - 1;
			}
			search >>= 1;
		}
		throw M2_ERROR("Implementation error in FindMostSignificantSetBit");
	}
}

m2::expected<m2::Exact> m2::Exact::Compose(const int32_t wholePart, const int32_t fractionalPart) {
	// PRECISION number of bits from the left of wholePart are gonna be thrown away
	if (wholePart != ((wholePart << PRECISION) >> PRECISION)) {
		return make_unexpected("Whole part contains an integer larger than Exact can accomadate");
	}
	// 32 - PRECISION number of bits from the left of fractionalPart are gonna be thrown away
	if (U(fractionalPart) != (U(fractionalPart) << (32 - PRECISION)) >> (32 - PRECISION)) {
		return make_unexpected("Fractional part has unexpected bits set");
	}
	return Exact{std::in_place, (wholePart << PRECISION) | fractionalPart};
}
m2::expected<m2::Exact> m2::Exact::ClosestExact(const std::string_view str) {
	// Find the decimal point
	int decimalPointIndex = -1;
	for (int i = 0; i < I(str.size()); ++i) {
		if (str[i] == '.') {
			decimalPointIndex = i;
			break;
		}
	}
	if (decimalPointIndex == -1) {
		return make_unexpected("Decimal point not found");
	}
	if (10 < decimalPointIndex) {
		return make_unexpected("Whole part of the number is too large");
	}

	static const int DECIMAL_DIGIT_SIGNIFICANCE[] = {
		0, // Significance of the decimal point
		1,
		10,
		100,
		1000,
		10000,
		100000,
		1000000,
		10000000,
		100000000,
		1000000000
	};
	static const int DECIMAL_PRECISION_POINT_TO_DECIMAL_8[] = {
		10000000,
		1000000,
		100000,
		10000,
		1000,
		100,
		10,
		1,
		0,
	};

	/// Calculate whole part separately, because this part isn't an approximation
	int32_t wholePart = 0;
	for (int i = 0; i < I(str.size()); ++i) {
		const char c = str[i];
		if ('0' <= c && c <= '9') {
			const int digitSignificance = decimalPointIndex - i;
			wholePart += (c - '0') * DECIMAL_DIGIT_SIGNIFICANCE[digitSignificance];
		} else if (c == '.') {
			/// Read fractional part as decimal raised by 8 digits
			int64_t fractionalPart8 = 0;
			for (int j = i + 1; j < I(str.size()); ++j) {
				const char f = str[j];
				if ('0' <= f && f <= '9') {
					const int fractionalDigitSignificance = j - decimalPointIndex - 1;
					fractionalPart8 += (f - '0') * DECIMAL_PRECISION_POINT_TO_DECIMAL_8[fractionalDigitSignificance];
				} else {
					return make_unexpected("Unexpected character: " + m2::ToString(f));
				}
			}
			/// Convert the fractional part from base(10) to base(2)
			return Compose(wholePart, I(fractionalPart8 * (1ll << PRECISION) / 100000000ll));
		} else {
			return make_unexpected("Unexpected character: " + m2::ToString(c));
		}
	}
	throw M2_ERROR("Implementation error: decimal point should have been encountered");
}

std::string m2::Exact::ToString() const {
	if constexpr (std::size(PRECISION_POINT_TO_DECIMAL_8) < PRECISION) {
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
		: IsNegative() ? Exact{std::in_place, -_value}.ToInteger() : ToInteger();
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
			fraction += PRECISION_POINT_TO_DECIMAL_8[i];
		}
	}
	// Append the fraction to the buffer
	snprintf(buffer.data() + seek, buffer.size() - seek, "%08d", fraction);

	return {buffer.data()};
}
std::string m2::Exact::ToFastString() const {
	std::array<char, 32> buffer{};
	std::ranges::fill(buffer, 0);
	snprintf(buffer.data(), buffer.size(), "%+016.8f", ToDouble());
	return {buffer.data()};
}
std::string m2::Exact::ToFastestString() const {
	std::array<char, 32> buffer{};
	std::ranges::fill(buffer, 0);
	snprintf(buffer.data(), buffer.size(), "%+016.08f", ToFloat());
	return {buffer.data()};
}

m2::Exact m2::Exact::Add(const Exact& other, const std::optional<Exact>& maxValue) const {
	return maxValue ? std::min(*this + other, *maxValue) : *this + other;
}
m2::Exact m2::Exact::Subtract(const Exact& other, const std::optional<Exact>& minValue) const {
	return minValue ? std::max(*this - other, *minValue) : *this - other;
}
m2::Exact m2::Exact::SquareRoot() const {
	if (IsNegative()) {
		throw M2_ERROR("Attempt to find the square root of a negative number");
	}
	if (IsZero()) {
		return *this;
	}

	// https://en.wikipedia.org/wiki/Square_root_algorithms#Binary_numeral_system_(base_2)

	auto num = int64_t{_value} << PRECISION;
	int64_t res = 0;

	// Start building the result from the greates power of four that's less than the value
	int64_t bit = 1ll << ((FindMostSignificantSetBit(_value) + PRECISION) / 2 * 2);
	for (; bit != 0; bit >>= 2) {
		const int64_t val = res + bit;
		res >>= 1;
		if (num >= val) {
			num -= val;
			res += bit;
		}
	}
	// Round the last digit up if necessary
	if (num > res) {
		res++;
	}
	return Exact{std::in_place, static_cast<int32_t>(res)};
}
m2::Exact m2::Exact::Round() const {
	// Make calculations in positive domain
	const auto abs = AbsoluteValue();
	const auto integerPart = abs._value & INTEGER_PART_MASK;
	const auto fractionalPart = abs._value & FRACTION_PART_MASK;
	constexpr auto fractionalHalfway = MOST_SIGNIFICANT_FRACTION_BIT_MASK;
	int32_t rounded;
	if (fractionalHalfway <= fractionalPart) {
		rounded = integerPart + LEAST_SIGNIFICANT_INTEGER_BIT_MASK;
	} else {
		rounded = integerPart;
	}
	return IsNegative() ? Exact{std::in_place, -rounded} : Exact{std::in_place, rounded};
}

void m2::Exact::ThrowIfOutOfBounds(const int i) {
	if (Max().ToInteger() < i) {
		throw M2_ERROR("Integer is more than what Exact can hold");
	}
	if (i < Min().ToInteger()) {
		throw M2_ERROR("Integer is less than what Exact can hold");
	}
}
void m2::Exact::ThrowIfOutOfBounds(const float f) {
	if (Max().ToFloat() < f) {
		throw M2_ERROR("Float is more than what Exact can hold");
	}
	if (f < Min().ToFloat()) {
		throw M2_ERROR("Float is less than what Exact can hold");
	}
}
void m2::Exact::ThrowIfOutOfBounds(const double d) {
	if (Max().ToDouble() < d) {
		throw M2_ERROR("Double is more than what Exact can hold");
	}
	if (d < Min().ToDouble()) {
		throw M2_ERROR("Double is less than what Exact can hold");
	}
}

std::string m2::ToString(const Exact& f) {
	return f.ToString();
}
