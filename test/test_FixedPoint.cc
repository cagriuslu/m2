#include <gtest/gtest.h>
#include <m2/math/FixedPoint.h>
#include <bitset>

using namespace m2;

TEST(FixedPoint, constructor) {
	EXPECT_EQ(FixedPoint{}.ToRawValue(), 0b0);
	EXPECT_EQ(FixedPoint{1}.ToRawValue(), 0b00000000'00000000'01000000'00000000);
	EXPECT_EQ(FixedPoint{-1}.ToRawValue(), 0b11111111'11111111'11000000'00000000);
	EXPECT_NO_THROW(FixedPoint{131071});
	EXPECT_ANY_THROW(FixedPoint{131072});
	EXPECT_NO_THROW(FixedPoint{-131072});
	EXPECT_ANY_THROW(FixedPoint{-131073});
	EXPECT_EQ(FixedPoint{1.0f}.ToRawValue(), 0b00000000'00000000'01000000'00000000);
	EXPECT_EQ(FixedPoint{1.1f}.ToRawValue(), 0b00000000'00000000'01000110'01100110);
	EXPECT_EQ(FixedPoint{-1.0f}.ToRawValue(), 0b11111111'11111111'11000000'00000000);
	EXPECT_EQ(FixedPoint{-1.1f}.ToRawValue(), 0b11111111'11111111'10111001'10011010);
	EXPECT_EQ(FixedPoint{1.0}.ToRawValue(), 0b00000000'00000000'01000000'00000000);
	EXPECT_EQ(FixedPoint{-1.0}.ToRawValue(), 0b11111111'11111111'11000000'00000000);
	EXPECT_NO_THROW(FixedPoint{131071.99993896}); // This is the highest number that can be represented
	EXPECT_NO_THROW(FixedPoint{-131072.0}); // This is the lowest number that can be represented
}

TEST(FixedPoint, attributes) {
	EXPECT_EQ(FixedPoint::Zero().ToRawValue(), 0b0);
	EXPECT_EQ(FixedPoint::Max().ToRawValue(), 0b01111111'11111111'11111111'11111111);
	EXPECT_EQ(FixedPoint::Min().ToRawValue(), 0b10000000'00000000'00000000'00000000);
	EXPECT_EQ(FixedPoint::MaxInteger().ToRawValue(), 0b01111111'11111111'11000000'00000000);
	EXPECT_EQ(FixedPoint::MinInteger().ToRawValue(), 0b10000000'00000000'00000000'00000000);
	EXPECT_EQ(FixedPoint::IntegerPartMask(), 0b11111111'11111111'11000000'00000000);
	EXPECT_EQ(FixedPoint::FractionalPartMask(), 0b00000000'00000000'00111111'11111111);
}

TEST(FixedPoint, accessors) {
	EXPECT_TRUE(FixedPoint{}.IsZero());
	EXPECT_FALSE(FixedPoint{1}.IsZero());
	EXPECT_FALSE(FixedPoint{-1}.IsZero());
	EXPECT_FALSE(FixedPoint{131071}.IsZero());
	EXPECT_FALSE(FixedPoint{-131072}.IsZero());

	EXPECT_FALSE(FixedPoint{}.IsPositive());
	EXPECT_TRUE(FixedPoint{1}.IsPositive());
	EXPECT_FALSE(FixedPoint{-1}.IsPositive());

	EXPECT_FALSE(FixedPoint{}.IsNegative());
	EXPECT_FALSE(FixedPoint{1}.IsNegative());
	EXPECT_TRUE(FixedPoint{-1}.IsNegative());

	EXPECT_EQ(FixedPoint{}.ToInteger(), 0);
	EXPECT_EQ(FixedPoint{1}.ToInteger(), 1);
	EXPECT_EQ(FixedPoint{-1}.ToInteger(), -1);
	EXPECT_EQ(FixedPoint{131071}.ToInteger(), 131071);
	EXPECT_EQ(FixedPoint{-131072}.ToInteger(), -131072);
	EXPECT_EQ(FixedPoint::Max().ToInteger(), 131071);
	EXPECT_EQ(FixedPoint::Min().ToInteger(), -131072);

	EXPECT_EQ(FixedPoint{}.ToFloat(), 0.0f);
	EXPECT_EQ(FixedPoint{1}.ToFloat(), 1.0f);
	EXPECT_EQ(FixedPoint{-1}.ToFloat(), -1.0f);
	EXPECT_EQ(FixedPoint{131071}.ToFloat(), 131071.0f);
	EXPECT_EQ(FixedPoint{131070}.ToFloat(), 131070.0f);
	EXPECT_EQ(FixedPoint{-131072}.ToFloat(), -131072.0f);
	EXPECT_EQ(FixedPoint{-131071}.ToFloat(), -131071.0f);
	// The result is accurate close to the origin
	EXPECT_EQ(FixedPoint{1234.54f}.ToFloat(), 1234.54f);
	EXPECT_EQ(FixedPoint{1234.56f}.ToFloat(), 1234.56f);
	EXPECT_EQ(FixedPoint{12345.6f}.ToFloat(), 12345.6f);
	EXPECT_EQ(FixedPoint{123456.0f}.ToFloat(), 123456.0f);
	EXPECT_EQ(FixedPoint{123456.0012f}.ToFloat(), 123456.0f);
	// The result is not accurate far from origin

	EXPECT_EQ(FixedPoint{}.ToDouble(), 0.0);
	EXPECT_EQ(FixedPoint{1}.ToDouble(), 1.0);
	EXPECT_EQ(FixedPoint{-1}.ToDouble(), -1.0);
	EXPECT_EQ(FixedPoint{131071}.ToDouble(), 131071.0);
	EXPECT_EQ(FixedPoint{-131072}.ToDouble(), -131072.0);

	EXPECT_EQ((FixedPoint{std::in_place, 0x73a8fcd3}.ToRawValue()), 0x73a8fcd3);

	EXPECT_EQ((FixedPoint{std::in_place, 0x00000001}.ToString()), "+000000.00006104");
	EXPECT_EQ((FixedPoint{-0.00006104}.ToString()), "-000000.00006104");
	EXPECT_EQ((FixedPoint{std::in_place, 0x7FFFFFFF}.ToString()), "+131071.99993896");
	EXPECT_EQ((FixedPoint{std::in_place, static_cast<int>(0x80000000)}.ToString()), "-131072.00000000");
	EXPECT_EQ((FixedPoint{std::in_place, static_cast<int>(0x80000001)}.ToString()), "-131071.99993896");

	EXPECT_EQ((FixedPoint{std::in_place, 0x00000001}.ToFastString()), "+000000.00006104");
	EXPECT_EQ((FixedPoint{-0.00006104}.ToFastString()), "-000000.00006104");
	EXPECT_EQ((FixedPoint{std::in_place, 0x7FFFFFFF}.ToFastString()), "+131071.99993896");
	EXPECT_EQ((FixedPoint{std::in_place, static_cast<int>(0x80000000)}.ToFastString()), "-131072.00000000");
	EXPECT_EQ((FixedPoint{std::in_place, static_cast<int>(0x80000001)}.ToFastString()), "-131071.99993896");

	// Fastest string is less accurate
	EXPECT_EQ((FixedPoint{std::in_place, 0x00000001}.ToFastestString()), "+000000.00006104");
	EXPECT_EQ((FixedPoint{-0.00006104}.ToFastestString()), "-000000.00006104");
	// EXPECT_EQ((FixedPoint{std::in_place, 0x7FFFFFFF}.ToFastestString()), "+131071.99993896");
	EXPECT_EQ((FixedPoint{std::in_place, static_cast<int>(0x80000000)}.ToFastestString()), "-131072.00000000");
	// EXPECT_EQ((FixedPoint{std::in_place, static_cast<int>(0x80000001)}.ToFastestString()), "-131071.99993896");
}

TEST(FixedPoint, modifiers) {
	std::cout << FixedPoint{1.2515}.ToString() << std::endl; // Closest number is 1.25152588
	std::cout << FixedPoint{2.6362}.ToString() << std::endl; // Closest number is 2.63623046
	std::cout << FixedPoint{3.2993106463}.ToString() << std::endl; // Multiplication of the closest numbers
	std::cout << (FixedPoint{1.2515} * FixedPoint{2.6362}).ToString() << std::endl; // Expectation: 3.2992043
	EXPECT_EQ((FixedPoint{1.2515} * FixedPoint{2.6362}).ToString(), "+000003.29925537"); // Close enough

	std::cout << FixedPoint{10.1946}.ToString() << std::endl; // Closest number is 10.19458007
	std::cout << FixedPoint{70.8069}.ToString() << std::endl; // Closest number is 70.80688476
	std::cout << FixedPoint{721.8464561931}.ToString() << std::endl; // Multiplication of the closest numbers
	std::cout << (FixedPoint{10.1946} * FixedPoint{70.8069}).ToString() << std::endl; // Expectation: 721.84802274
	EXPECT_EQ((FixedPoint{10.1946} * FixedPoint{70.8069}).ToString(), "+000721.84643554"); // Close enough
}
