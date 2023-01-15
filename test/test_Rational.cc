#include <gtest/gtest.h>
#include <m2/Rational.h>

using namespace m2;

TEST(Rational, construct) {
	Rational r1;
	EXPECT_EQ(r1.n(), 0);
	EXPECT_EQ(r1.d(), 1);

	Rational r2{1234};
	EXPECT_EQ(r2.n(), 1234);
	EXPECT_EQ(r2.d(), 1);

	Rational r3{12, 24};
	EXPECT_EQ(r3.n(), 12);
	EXPECT_EQ(r3.d(), 24);

	auto r4 = Rational{12, 24}.simplify();
	EXPECT_EQ(r4.n(), 1);
	EXPECT_EQ(r4.d(), 2);
}

TEST(Rational, operators) {
	Rational r1{1, 2};
	Rational r2{1, 4};
	auto r3 = r1 + r2;
	Rational r4{3, 4};
	EXPECT_EQ(r3.n(), r4.n());
	EXPECT_EQ(r3.d(), r4.d());
}
