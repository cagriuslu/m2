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
		Rational r{0.1234567};
		EXPECT_EQ(r.n(), 1234567);
		EXPECT_EQ(r.d(), 10000000);
	}
	{
		Rational r{0.123456};
		EXPECT_EQ(r.n(), 1929);
		EXPECT_EQ(r.d(), 15625);
	}
	{
		Rational r{0.12345};
		EXPECT_EQ(r.n(), 2469);
		EXPECT_EQ(r.d(), 20000);
	}
	{
		Rational r{0.1234};
		EXPECT_EQ(r.n(), 617);
		EXPECT_EQ(r.d(), 5000);
	}
	{
		Rational r{1.234};
		EXPECT_EQ(r.n(), 617);
		EXPECT_EQ(r.d(), 500);
	}
	{
		Rational r{12.34};
		EXPECT_EQ(r.n(), 617);
		EXPECT_EQ(r.d(), 50);
	}
	{
		Rational r{123.4};
		EXPECT_EQ(r.n(), 617);
		EXPECT_EQ(r.d(), 5);
	}
	{
		Rational r{1234.5};
		EXPECT_EQ(r.n(), 2469);
		EXPECT_EQ(r.d(), 2);
	}
	{
		Rational r{12345.6};
		EXPECT_EQ(r.n(), 61728);
		EXPECT_EQ(r.d(), 5);
	}
	{
		Rational r{123456.7};
		EXPECT_EQ(r.n(), 1234567);
		EXPECT_EQ(r.d(), 10);
	}
	{
		Rational r{1234567.8};
		EXPECT_EQ(r.n(), 6172839);
		EXPECT_EQ(r.d(), 5);
	}
	{
		Rational r{12345678.9};
		EXPECT_EQ(r.n(), 123456789);
		EXPECT_EQ(r.d(), 10);
	}
	{
		Rational r{123456789.1};
		EXPECT_EQ(r.n(), 1234567891);
		EXPECT_EQ(r.d(), 10);
	}
	{
		Rational r{1234567891.2};
		EXPECT_EQ(r.n(), 6172839456);
		EXPECT_EQ(r.d(), 5);
	}
	{
		Rational r{12345678912.3};
		EXPECT_EQ(r.n(), 123456789123);
		EXPECT_EQ(r.d(), 10);
	}
	{
		Rational r{123456789123.4};
		EXPECT_EQ(r.n(), 617283945617);
		EXPECT_EQ(r.d(), 5);
	}
	{
		Rational r{1234567891234.5};
		EXPECT_EQ(r.n(), 2469135782469);
		EXPECT_EQ(r.d(), 2);
	}
	{
		Rational r{12345678912345.6};
		EXPECT_EQ(r.n(), 61728394561728);
		EXPECT_EQ(r.d(), 5);
	}
	{
		// This one overflows
		Rational r{123456789123456.7};
		EXPECT_EQ(r.n(), 123456789123457);
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
