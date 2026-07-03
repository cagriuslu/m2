#include <gtest/gtest.h>
#include <m2/common/Math.h>
#include <m2/common/Constants.h>

using namespace m2;

TEST(Math, FloatComparisons) {
	EXPECT_TRUE(IsEqual(1.0f, 1.05f, 0.1f));
	EXPECT_FALSE(IsEqual(1.0f, 1.2f, 0.1f));
	EXPECT_TRUE(IsNotEqual(1.0f, 1.2f, 0.1f));
	EXPECT_TRUE(IsLess(1.0f, 2.0f, 0.1f));
	EXPECT_FALSE(IsLess(1.0f, 1.05f, 0.1f));
	EXPECT_TRUE(IsZero(0.05f, 0.1f));
	EXPECT_TRUE(IsPositive(1.0f, 0.1f));
	EXPECT_TRUE(IsNegative(-1.0f, 0.1f));
	EXPECT_TRUE(IsOne(1.0f, 0.1f));
}

TEST(Math, DegreesAndRadians) {
	EXPECT_FLOAT_EQ(ToRadians(180.0f), PI);
	EXPECT_FLOAT_EQ(ToRadians(90), PI_DIV2);
	EXPECT_FLOAT_EQ(ToDegrees(PI), 180.0f);
}

TEST(Math, ClampRadians) {
	// -PI/2 wraps into [0, 2*PI) as 3*PI/2
	EXPECT_NEAR(ClampRadiansTo2Pi(-PI_DIV2), PI_MUL3_DIV2, 0.0001f);
	EXPECT_NEAR(ClampRadiansTo2Pi(PI_DIV2), PI_DIV2, 0.0001f);
	// 3*PI/2 wraps into [-PI, PI) as -PI/2
	EXPECT_NEAR(ClampRadiansToPi(PI_MUL3_DIV2), -PI_DIV2, 0.0001f);
	EXPECT_NEAR(ClampRadiansToPi(PI_DIV2), PI_DIV2, 0.0001f);
}

TEST(Math, AngleDifferences) {
	EXPECT_NEAR(AngleAbsoluteDifference(0.0f, PI_DIV2), PI_DIV2, 0.0001f);
	// The absolute difference takes the shorter way around the circle
	EXPECT_NEAR(AngleAbsoluteDifference(0.1f, PI_MUL2 - 0.1f), 0.2f, 0.0001f);
	EXPECT_NEAR(AngleDifference(PI_DIV2, 0.0f), PI_DIV2, 0.0001f);
	EXPECT_NEAR(AngleDifference(0.0f, PI_DIV2), -PI_DIV2, 0.0001f);
}

TEST(Math, ToFloat) {
	EXPECT_FLOAT_EQ(ToFloat(std::string{"3.5"}).value(), 3.5f);
	EXPECT_FALSE(ToFloat(std::string{"abc"}).has_value());
}

TEST(Math, RoundToBin) {
	// With 4 bins per unit, the fractional part snaps to the nearest 0.25
	EXPECT_FLOAT_EQ(RoundToBin(1.2f, 4), 1.25f);
	EXPECT_FLOAT_EQ(RoundToBin(1.6f, 4), 1.5f);
	EXPECT_FLOAT_EQ(RoundToBin(2.0f, 4), 2.0f);
}

TEST(Math, Normalize) {
	EXPECT_FLOAT_EQ(Normalize(5.0f, 0.0f, 10.0f), 0.5f);
	EXPECT_FLOAT_EQ(Normalize(0.0f, 0.0f, 10.0f), 0.0f);
	EXPECT_FLOAT_EQ(Normalize(10.0f, 0.0f, 10.0f), 1.0f);
	// Values outside [min,max] scale beyond [0,1]
	EXPECT_FLOAT_EQ(Normalize(15.0f, 0.0f, 10.0f), 1.5f);
}

TEST(Math, Lerp) {
	EXPECT_FLOAT_EQ(Lerp(0.0f, 10.0f, 0.5f), 5.0f);
	EXPECT_FLOAT_EQ(Lerp(2.0f, 4.0f, 0.0f), 2.0f);
	EXPECT_FLOAT_EQ(Lerp(2.0f, 4.0f, 1.0f), 4.0f);
}

TEST(Math, AtLeastAtMostZero) {
	EXPECT_EQ(AtLeastZero(-5), 0);
	EXPECT_EQ(AtLeastZero(5), 5);
	EXPECT_EQ(AtMostZero(5), 0);
	EXPECT_EQ(AtMostZero(-5), -5);
}
