#include <gtest/gtest.h>
#include <m2/math/primitives/Fixed.h>
#include <bitset>

using namespace m2;

TEST(Fixed, constructors) {
	EXPECT_EQ(Fixed{}.ToRawValue(), 0b0);
	EXPECT_EQ(Fixed{1}.ToRawValue(), 0b00000000'00000000'01000000'00000000);
	EXPECT_EQ(Fixed{-1}.ToRawValue(), 0b11111111'11111111'11000000'00000000);
	EXPECT_NO_THROW(Fixed{131071});
	EXPECT_ANY_THROW(Fixed{131072});
	EXPECT_NO_THROW(Fixed{-131072});
	EXPECT_ANY_THROW(Fixed{-131073});
	EXPECT_EQ(Fixed{1.0f}.ToRawValue(), 0b00000000'00000000'01000000'00000000);
	EXPECT_EQ(Fixed{1.1f}.ToRawValue(), 0b00000000'00000000'01000110'01100110);
	EXPECT_EQ(Fixed{-1.0f}.ToRawValue(), 0b11111111'11111111'11000000'00000000);
	EXPECT_EQ(Fixed{-1.1f}.ToRawValue(), 0b11111111'11111111'10111001'10011010);
	EXPECT_EQ(Fixed{1.0}.ToRawValue(), 0b00000000'00000000'01000000'00000000);
	EXPECT_EQ(Fixed{-1.0}.ToRawValue(), 0b11111111'11111111'11000000'00000000);
	EXPECT_EQ(Fixed{1.5}.ToRawValue(), 0b00000000'00000000'01100000'00000000);
	EXPECT_EQ(Fixed{3.75}.ToRawValue(), 0b00000000'00000000'11110000'00000000);
	EXPECT_EQ(Fixed{7.875}.ToRawValue(), 0b00000000'00000001'11111000'00000000);
	EXPECT_EQ(Fixed{15.9375}.ToRawValue(), 0b00000000'00000011'11111100'00000000);
	EXPECT_EQ(Fixed{31.96875}.ToRawValue(), 0b00000000'00000111'11111110'00000000);
	EXPECT_EQ(Fixed{63.984375}.ToRawValue(), 0b00000000'00001111'11111111'00000000);
	EXPECT_EQ(Fixed{127.9921875}.ToRawValue(), 0b00000000'00011111'11111111'10000000);
	EXPECT_EQ(Fixed{255.99609375}.ToRawValue(), 0b00000000'00111111'11111111'11000000);
	EXPECT_EQ(Fixed{511.99804688}.ToRawValue(), 0b00000000'01111111'11111111'11100000);
	EXPECT_EQ(Fixed{1023.99902344}.ToRawValue(), 0b00000000'11111111'11111111'11110000);
	EXPECT_EQ(Fixed{2047.99951172}.ToRawValue(), 0b00000001'11111111'11111111'11111000);
	EXPECT_EQ(Fixed{4095.99975586}.ToRawValue(), 0b00000011'11111111'11111111'11111100);
	// Number bigger than this cannot be represented by double accurately
	EXPECT_NO_THROW(Fixed{131071.99993896}); // This is the highest number that can be represented
	EXPECT_NO_THROW(Fixed{-131072.0}); // This is the lowest number that can be represented

	EXPECT_EQ(Fixed::FromProtobufRepresentation(0).ToString(), "+000000.00000000");
	EXPECT_EQ(Fixed::FromProtobufRepresentation(1'000'000ll).ToString(), "+000001.00000000");
	EXPECT_EQ(Fixed::FromProtobufRepresentation(-1'000'000ll).ToString(), "-000001.00000000");
	EXPECT_EQ(Fixed::FromProtobufRepresentation(1'234'000'000ll).ToString(), "+001234.00000000");
	EXPECT_EQ(Fixed::FromProtobufRepresentation(-1'234'000'000ll).ToString(), "-001234.00000000");
	EXPECT_ANY_THROW(Fixed::FromProtobufRepresentation(132'000'000'000ll));
	EXPECT_ANY_THROW(Fixed::FromProtobufRepresentation(-132'000'000'000ll));
	EXPECT_EQ(Fixed::FromProtobufRepresentation(12'340'000ll).ToRawValue(), 0b00000000'00000011'00010101'11000010);
	EXPECT_EQ(Fixed::FromProtobufRepresentation(1'234'567'800ll).ToString(), "+001234.56774902");
	EXPECT_EQ(Fixed::FromProtobufRepresentation(-1'234'567'800ll).ToString(), "-001234.56774902");
}

TEST(Fixed, operators) {
	EXPECT_EQ((Fixed{1.5} + Fixed{3.75}).ToString(), "+000005.25000000");
	EXPECT_EQ((Fixed{1.5} + Fixed{3.75}), Fixed{5.25});

	EXPECT_EQ((Fixed{3.75} - Fixed{1.5}).ToString(), "+000002.25000000");
	EXPECT_EQ((Fixed{3.75} - Fixed{1.5}), Fixed{2.25});

	std::cout << Fixed{1.2515}.ToString() << std::endl; // Closest number is 1.25152588
	std::cout << Fixed{2.6362}.ToString() << std::endl; // Closest number is 2.63623046
	std::cout << Fixed{3.2993106463}.ToString() << std::endl; // Multiplication of the closest numbers
	std::cout << (Fixed{1.2515} * Fixed{2.6362}).ToString() << std::endl; // Expectation: 3.2992043
	EXPECT_EQ((Fixed{1.2515} * Fixed{2.6362}).ToString(), "+000003.29925538"); // Close enough

	std::cout << Fixed{10.1946}.ToString() << std::endl; // Closest number is 10.19458007
	std::cout << Fixed{70.8069}.ToString() << std::endl; // Closest number is 70.80688476
	std::cout << Fixed{721.8464561931}.ToString() << std::endl; // Multiplication of the closest numbers
	std::cout << (Fixed{10.1946} * Fixed{70.8069}).ToString() << std::endl; // Expectation: 721.84802274
	EXPECT_EQ((Fixed{10.1946} * Fixed{70.8069}).ToString(), "+000721.84643555"); // Close enough
}

TEST(Fixed, attributes) {
	EXPECT_EQ(Fixed::Zero().ToRawValue(), 0b0);
	EXPECT_EQ(Fixed::Max().ToRawValue(), 0b01111111'11111111'11111111'11111111);
	EXPECT_EQ(Fixed::Min().ToRawValue(), 0b10000000'00000000'00000000'00000000);
	EXPECT_EQ(Fixed::MaxInteger().ToRawValue(), 0b01111111'11111111'11000000'00000000);
	EXPECT_EQ(Fixed::MinInteger().ToRawValue(), 0b10000000'00000000'00000000'00000000);
}

TEST(Fixed, accessors) {
	EXPECT_TRUE(Fixed{}.IsZero());
	EXPECT_FALSE(Fixed{1}.IsZero());
	EXPECT_FALSE(Fixed{-1}.IsZero());
	EXPECT_FALSE(Fixed{131071}.IsZero());
	EXPECT_FALSE(Fixed{-131072}.IsZero());

	EXPECT_FALSE(Fixed{}.IsPositive());
	EXPECT_TRUE(Fixed{1}.IsPositive());
	EXPECT_FALSE(Fixed{-1}.IsPositive());

	EXPECT_FALSE(Fixed{}.IsNegative());
	EXPECT_FALSE(Fixed{1}.IsNegative());
	EXPECT_TRUE(Fixed{-1}.IsNegative());

	EXPECT_EQ(Fixed{}.ToInteger(), 0);
	EXPECT_EQ(Fixed{1}.ToInteger(), 1);
	EXPECT_EQ(Fixed{-1}.ToInteger(), -1);
	EXPECT_EQ(Fixed{131071}.ToInteger(), 131071);
	EXPECT_EQ(Fixed{-131072}.ToInteger(), -131072);
	EXPECT_EQ(Fixed::Max().ToInteger(), 131071);
	EXPECT_EQ(Fixed::Min().ToInteger(), -131072);

	EXPECT_EQ(Fixed{}.ToFloat(), 0.0f);
	EXPECT_EQ(Fixed{1}.ToFloat(), 1.0f);
	EXPECT_EQ(Fixed{-1}.ToFloat(), -1.0f);
	EXPECT_EQ(Fixed{131071}.ToFloat(), 131071.0f);
	EXPECT_EQ(Fixed{131070}.ToFloat(), 131070.0f);
	EXPECT_EQ(Fixed{-131072}.ToFloat(), -131072.0f);
	EXPECT_EQ(Fixed{-131071}.ToFloat(), -131071.0f);
	// The result is accurate close to the origin
	EXPECT_EQ(Fixed{1234.54f}.ToFloat(), 1234.54f);
	EXPECT_EQ(Fixed{1234.56f}.ToFloat(), 1234.56f);
	EXPECT_EQ(Fixed{12345.6f}.ToFloat(), 12345.6f);
	EXPECT_EQ(Fixed{123456.0f}.ToFloat(), 123456.0f);
	EXPECT_EQ(Fixed{123456.0012f}.ToFloat(), 123456.0f);
	// The result is not accurate far from origin

	EXPECT_EQ(Fixed{}.ToDouble(), 0.0);
	EXPECT_EQ(Fixed{1}.ToDouble(), 1.0);
	EXPECT_EQ(Fixed{-1}.ToDouble(), -1.0);
	EXPECT_EQ(Fixed{131071}.ToDouble(), 131071.0);
	EXPECT_EQ(Fixed{-131072}.ToDouble(), -131072.0);

	EXPECT_EQ((Fixed{std::in_place, 0x73a8fcd3}.ToRawValue()), 0x73a8fcd3);

	EXPECT_EQ((Fixed{std::in_place, 0x00000001}.ToString()), "+000000.00006104");
	EXPECT_EQ((Fixed{-0.00006104}.ToString()), "-000000.00006104");
	EXPECT_EQ((Fixed{std::in_place, 0x7FFFFFFF}.ToString()), "+131071.99993897");
	EXPECT_EQ((Fixed{std::in_place, static_cast<int>(0x80000000)}.ToString()), "-131072.00000000");
	EXPECT_EQ((Fixed{std::in_place, static_cast<int>(0x80000001)}.ToString()), "-131071.99993897");

	EXPECT_EQ((Fixed{std::in_place, 0x00000001}.ToFastString()), "+000000.00006104");
	EXPECT_EQ((Fixed{-0.00006104}.ToFastString()), "-000000.00006104");
	EXPECT_EQ((Fixed{std::in_place, 0x7FFFFFFF}.ToFastString()), "+131071.99993896");
	EXPECT_EQ((Fixed{std::in_place, static_cast<int>(0x80000000)}.ToFastString()), "-131072.00000000");
	EXPECT_EQ((Fixed{std::in_place, static_cast<int>(0x80000001)}.ToFastString()), "-131071.99993896");

	// Fastest string is less accurate
	EXPECT_EQ((Fixed{std::in_place, 0x00000001}.ToFastestString()), "+000000.00006104");
	EXPECT_EQ((Fixed{-0.00006104}.ToFastestString()), "-000000.00006104");
	EXPECT_EQ((Fixed{std::in_place, 0x7FFFFFFF}.ToFastestString()), "+131072.00000000");
	EXPECT_EQ((Fixed{std::in_place, static_cast<int>(0x80000000)}.ToFastestString()), "-131072.00000000");
	EXPECT_EQ((Fixed{std::in_place, static_cast<int>(0x80000001)}.ToFastestString()), "-131072.00000000");
}
