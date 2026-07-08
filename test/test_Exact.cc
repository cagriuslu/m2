#include <gtest/gtest.h>
#include <m2/common/math/Exact.h>

using namespace m2;

TEST(Exact, constructors) {
	EXPECT_EQ(Exact{}.ToRawValue(), 0b0);
	EXPECT_EQ(Exact{1}.ToRawValue(), 0b00000000'00000000'01000000'00000000);
	EXPECT_EQ(Exact{-1}.ToRawValue(), 0b11111111'11111111'11000000'00000000);
	EXPECT_NO_THROW(Exact{131071});
	EXPECT_ANY_THROW(Exact{131072});
	EXPECT_NO_THROW(Exact{-131072});
	EXPECT_ANY_THROW(Exact{-131073});
	// Float and double to Exact conversion is forbidden in deterministic games
	EXPECT_ANY_THROW(Exact{1.0f});
	EXPECT_ANY_THROW(Exact{1.0});

	EXPECT_EQ(Exact::Compose(0, 0)->ToRawValue(), 0b00000000'00000000'00000000'00000000);
	EXPECT_EQ(Exact::Compose(1, 0)->ToRawValue(), 0b00000000'00000000'01000000'00000000);
	EXPECT_EQ(Exact::Compose(1, Exact::RAW_0P5)->ToRawValue(), 0b00000000'00000000'01100000'00000000);
	EXPECT_EQ(Exact::Compose(1024, Exact::RAW_1DIV1024)->ToRawValue(), 0b00000001'00000000'00000000'00010000);
	EXPECT_EQ(Exact::Compose(65536, 0)->ToRawValue(), 0b01000000'00000000'00000000'00000000);
	EXPECT_EQ(Exact::Compose(131071, 0)->ToRawValue(), 0b01111111'11111111'11000000'00000000);
	EXPECT_EQ(Exact::Compose(-131072, 0)->ToRawValue(), 0b10000000'00000000'00000000'00000000);
	EXPECT_FALSE(Exact::Compose(131072, 0));
	EXPECT_FALSE(Exact::Compose(-131073, 0));

	EXPECT_EQ(Exact::ClosestExact("123.5")->ToString(), "+000123.50000000");
	EXPECT_EQ("123.25"_closest_exact->ToString(), "+000123.25000000");
	EXPECT_EQ("123.75"_closest_exact->ToRawValue(), 0b00000000'00011110'11110000'00000000);
	EXPECT_EQ("1.99902344"_closest_exact->ToRawValue(), 0b00000000'00000000'01111111'11110000);

	// Some random number
	EXPECT_EQ("12345.7254247"_closest_exact->ToString(), "+012345.72540284");
	// Its binary representation
	EXPECT_EQ("12345.7254247"_closest_exact->ToRawValue(), 0b00001100'00001110'01101110'01101101);
	// Next number is already too far from the initial number, nice!
	EXPECT_EQ((Exact{std::in_place, 0b00001100'00001110'01101110'01101110}.ToString()), "+012345.72546387");
}

TEST(Exact, operators) {
	EXPECT_EQ((*"1.5"_closest_exact + *"3.75"_closest_exact).ToString(), "+000005.25000000");
	EXPECT_EQ((*"1.5"_closest_exact + *"3.75"_closest_exact), *"5.25"_closest_exact);

	EXPECT_EQ((*"3.75"_closest_exact - *"1.5"_closest_exact).ToString(), "+000002.25000000");
	EXPECT_EQ((*"3.75"_closest_exact - *"1.5"_closest_exact), *"2.25"_closest_exact);

	std::cout << (*"1.2515"_closest_exact).ToString() << std::endl; // Closest number is 1.25146484
	std::cout << (*"2.6362"_closest_exact).ToString() << std::endl; // Closest number is 2.63616944
	std::cout << (*"3.2990733662"_closest_exact).ToString() << std::endl; // Multiplication of the closest numbers
	std::cout << (*"1.2515"_closest_exact * *"2.6362"_closest_exact).ToString() << std::endl; // Expectation: 3.29907337
	EXPECT_EQ((*"1.2515"_closest_exact * *"2.6362"_closest_exact).ToString(), "+000003.29907227"); // Close enough

	std::cout << (*"10.1946"_closest_exact).ToString() << std::endl; // Closest number is 10.19458008
	std::cout << (*"70.8069"_closest_exact).ToString() << std::endl; // Closest number is 70.80688477
	std::cout << (*"721.8464561931"_closest_exact).ToString() << std::endl; // Multiplication of the closest numbers
	std::cout << (*"10.1946"_closest_exact * *"70.8069"_closest_exact).ToString() << std::endl; // Expectation: 721.84802274
	EXPECT_EQ((*"10.1946"_closest_exact * *"70.8069"_closest_exact).ToString(), "+000721.84643555"); // Close enough
}

TEST(Exact, attributes) {
	EXPECT_EQ(Exact::Zero().ToRawValue(), 0b0);
	EXPECT_EQ(Exact::Max().ToRawValue(), 0b01111111'11111111'11111111'11111111);
	EXPECT_EQ(Exact::Min().ToRawValue(), 0b10000000'00000000'00000000'00000000);
	EXPECT_EQ(Exact::MaxInteger().ToRawValue(), 0b01111111'11111111'11000000'00000000);
	EXPECT_EQ(Exact::MinInteger().ToRawValue(), 0b10000000'00000000'00000000'00000000);
}

TEST(Exact, accessors) {
	EXPECT_TRUE(Exact{}.IsZero());
	EXPECT_FALSE(Exact{1}.IsZero());
	EXPECT_FALSE(Exact{-1}.IsZero());
	EXPECT_FALSE(Exact{131071}.IsZero());
	EXPECT_FALSE(Exact{-131072}.IsZero());

	EXPECT_FALSE(Exact{}.IsPositive());
	EXPECT_TRUE(Exact{1}.IsPositive());
	EXPECT_FALSE(Exact{-1}.IsPositive());

	EXPECT_FALSE(Exact{}.IsNegative());
	EXPECT_FALSE(Exact{1}.IsNegative());
	EXPECT_TRUE(Exact{-1}.IsNegative());

	EXPECT_EQ(Exact{}.ToInteger(), 0);
	EXPECT_EQ(Exact{1}.ToInteger(), 1);
	EXPECT_EQ(Exact{-1}.ToInteger(), -1);
	EXPECT_EQ(Exact{131071}.ToInteger(), 131071);
	EXPECT_EQ(Exact{-131072}.ToInteger(), -131072);
	EXPECT_EQ(Exact::Max().ToInteger(), 131071);
	EXPECT_EQ(Exact::Min().ToInteger(), -131072);

	EXPECT_EQ(Exact{}.ToFloat(), 0.0f);
	EXPECT_EQ(Exact{1}.ToFloat(), 1.0f);
	EXPECT_EQ(Exact{-1}.ToFloat(), -1.0f);
	EXPECT_EQ(Exact{131071}.ToFloat(), 131071.0f);
	EXPECT_EQ(Exact{131070}.ToFloat(), 131070.0f);
	EXPECT_EQ(Exact{-131072}.ToFloat(), -131072.0f);
	EXPECT_EQ(Exact{-131071}.ToFloat(), -131071.0f);

	EXPECT_EQ(Exact{}.ToDouble(), 0.0);
	EXPECT_EQ(Exact{1}.ToDouble(), 1.0);
	EXPECT_EQ(Exact{-1}.ToDouble(), -1.0);
	EXPECT_EQ(Exact{131071}.ToDouble(), 131071.0);
	EXPECT_EQ(Exact{-131072}.ToDouble(), -131072.0);

	EXPECT_EQ((Exact{std::in_place, 0x73a8fcd3}.ToRawValue()), 0x73a8fcd3);

	EXPECT_EQ((Exact{std::in_place, 0x00000001}.ToString()), "+000000.00006104");
	EXPECT_EQ((Exact{std::in_place, -1}.ToString()), "-000000.00006104");
	EXPECT_EQ((Exact{std::in_place, 0x7FFFFFFF}.ToString()), "+131071.99993897");
	EXPECT_EQ((Exact{std::in_place, static_cast<int>(0x80000000)}.ToString()), "-131072.00000000");
	EXPECT_EQ((Exact{std::in_place, static_cast<int>(0x80000001)}.ToString()), "-131071.99993897");

	EXPECT_EQ((Exact{std::in_place, 0x00000001}.ToFastString()), "+000000.00006104");
	EXPECT_EQ((Exact{std::in_place, -1}.ToFastString()), "-000000.00006104");
	EXPECT_EQ((Exact{std::in_place, 0x7FFFFFFF}.ToFastString()), "+131071.99993896");
	EXPECT_EQ((Exact{std::in_place, static_cast<int>(0x80000000)}.ToFastString()), "-131072.00000000");
	EXPECT_EQ((Exact{std::in_place, static_cast<int>(0x80000001)}.ToFastString()), "-131071.99993896");

	// Fastest string is less accurate
	EXPECT_EQ((Exact{std::in_place, 0x00000001}.ToFastestString()), "+000000.00006104");
	EXPECT_EQ((Exact{std::in_place, -1}.ToFastestString()), "-000000.00006104");
	EXPECT_EQ((Exact{std::in_place, 0x7FFFFFFF}.ToFastestString()), "+131072.00000000");
	EXPECT_EQ((Exact{std::in_place, static_cast<int>(0x80000000)}.ToFastestString()), "-131072.00000000");
	EXPECT_EQ((Exact{std::in_place, static_cast<int>(0x80000001)}.ToFastestString()), "-131072.00000000");
}

TEST(Exact, square_root) {
	EXPECT_EQ(Exact{0}.SquareRoot().ToString(), "+000000.00000000");
	EXPECT_EQ(Exact{2}.SquareRoot().ToString(), "+000001.41418457");
	EXPECT_EQ((*"123.456"_closest_exact).SquareRoot().ToString(), "+000011.11108398");
	EXPECT_EQ((*"12345.6789"_closest_exact).SquareRoot().ToString(), "+000111.11108398");
	EXPECT_EQ((*"99999.9999"_closest_exact).SquareRoot().ToString(), "+000316.22778320");
}

TEST(Exact, round) {
	EXPECT_EQ((*"3.4999"_closest_exact).Round(), Exact{3});
	EXPECT_EQ((*"3.5"_closest_exact).Round(), Exact{4});
	EXPECT_EQ((*"3.5001"_closest_exact).Round(), Exact{4});
	EXPECT_EQ((*"3.9999"_closest_exact).Round(), Exact{4});
	EXPECT_EQ(Exact{4}.Round(), Exact{4});
	EXPECT_EQ((*"4.0001"_closest_exact).Round(), Exact{4});
}
