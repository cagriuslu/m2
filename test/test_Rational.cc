#include <gtest/gtest.h>
#include <m2/Rational.h>

using namespace m2;

TEST(Rational, construct) {
	{
		Rational r;
		EXPECT_EQ(r.n(), 0);
		EXPECT_EQ(r.d(), 1);
	}
	{
		Rational r{1234};
		EXPECT_EQ(r.n(), 1234);
		EXPECT_EQ(r.d(), 1);
	}
	{
		Rational r{12, 24};
		EXPECT_EQ(r.n(), 12);
		EXPECT_EQ(r.d(), 24);
	}
	{
		Rational r{0.1234567f};
		EXPECT_EQ(r.n(), 1234567);
		EXPECT_EQ(r.d(), 10000000);
	}
	{
		Rational r{0.123456f};
		EXPECT_EQ(r.n(), 1929);
		EXPECT_EQ(r.d(), 15625);
	}
	{
		Rational r{0.12345f};
		EXPECT_EQ(r.n(), 2469);
		EXPECT_EQ(r.d(), 20000);
	}
	{
		Rational r{0.1234f};
		EXPECT_EQ(r.n(), 617);
		EXPECT_EQ(r.d(), 5000);
	}
	{
		Rational r{1.234f};
		EXPECT_EQ(r.n(), 617);
		EXPECT_EQ(r.d(), 500);
	}
	{
		Rational r{12.34f};
		EXPECT_EQ(r.n(), 617);
		EXPECT_EQ(r.d(), 50);
	}
	{
		Rational r{123.4f};
		EXPECT_EQ(r.n(), 617);
		EXPECT_EQ(r.d(), 5);
	}
	{
		Rational r{1234.5f};
		EXPECT_EQ(r.n(), 2469);
		EXPECT_EQ(r.d(), 2);
	}
	{
		Rational r{12345.6f};
		EXPECT_EQ(r.n(), 61728);
		EXPECT_EQ(r.d(), 5);
	}
	{
		Rational r{123456.7f};
		EXPECT_EQ(r.n(), 1234567);
		EXPECT_EQ(r.d(), 10);
	}
	{
		// This one overflows
		Rational r{1234567.8f};
		EXPECT_EQ(r.n(), 1234568);
		EXPECT_EQ(r.d(), 1);
	}
}

TEST(Rational, operators) {
	Rational r1{1, 2};
	Rational r2{1, 4};
	auto r3 = r1 + r2;
	EXPECT_EQ(r3.n(), 3);
	EXPECT_EQ(r3.d(), 4);
}

TEST(Rational, simplify) {
	auto r = Rational{12, 24}.simplify();
	EXPECT_EQ(r.n(), 1);
	EXPECT_EQ(r.d(), 2);
}
