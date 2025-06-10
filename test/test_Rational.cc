#include <gtest/gtest.h>
#include <m2/math/Rational.h>

using namespace m2;

TEST(Rational, construct) {
	{
		Rational r;
		EXPECT_EQ(r.GetN(), 0);
		EXPECT_EQ(r.GetD(), 1);
	}
	{
		Rational r{1234};
		EXPECT_EQ(r.GetN(), 1234);
		EXPECT_EQ(r.GetD(), 1);
	}
	{
		Rational r{12, 24};
		EXPECT_EQ(r.GetN(), 12);
		EXPECT_EQ(r.GetD(), 24);
	}
	{
		Rational r{0.1234567};
		EXPECT_EQ(r.GetN(), 1234567);
		EXPECT_EQ(r.GetD(), 10000000);
	}
	{
		Rational r{0.123456};
		EXPECT_EQ(r.GetN(), 1929);
		EXPECT_EQ(r.GetD(), 15625);
	}
	{
		Rational r{0.12345};
		EXPECT_EQ(r.GetN(), 2469);
		EXPECT_EQ(r.GetD(), 20000);
	}
	{
		Rational r{0.1234};
		EXPECT_EQ(r.GetN(), 617);
		EXPECT_EQ(r.GetD(), 5000);
	}
	{
		Rational r{1.234};
		EXPECT_EQ(r.GetN(), 617);
		EXPECT_EQ(r.GetD(), 500);
	}
	{
		Rational r{12.34};
		EXPECT_EQ(r.GetN(), 617);
		EXPECT_EQ(r.GetD(), 50);
	}
	{
		Rational r{123.4};
		EXPECT_EQ(r.GetN(), 617);
		EXPECT_EQ(r.GetD(), 5);
	}
	{
		Rational r{1234.5};
		EXPECT_EQ(r.GetN(), 2469);
		EXPECT_EQ(r.GetD(), 2);
	}
	{
		Rational r{12345.6};
		EXPECT_EQ(r.GetN(), 61728);
		EXPECT_EQ(r.GetD(), 5);
	}
	{
		Rational r{123456.7};
		EXPECT_EQ(r.GetN(), 1234567);
		EXPECT_EQ(r.GetD(), 10);
	}
	{
		Rational r{1234567.8};
		EXPECT_EQ(r.GetN(), 6172839);
		EXPECT_EQ(r.GetD(), 5);
	}
	{
		Rational r{12345678.9};
		EXPECT_EQ(r.GetN(), 123456789);
		EXPECT_EQ(r.GetD(), 10);
	}
	{
		Rational r{123456789.1};
		EXPECT_EQ(r.GetN(), 1234567891);
		EXPECT_EQ(r.GetD(), 10);
	}
	{
		Rational r{1234567891.2};
		EXPECT_EQ(r.GetN(), 6172839456);
		EXPECT_EQ(r.GetD(), 5);
	}
	{
		Rational r{12345678912.3};
		EXPECT_EQ(r.GetN(), 123456789123);
		EXPECT_EQ(r.GetD(), 10);
	}
	{
		Rational r{123456789123.4};
		EXPECT_EQ(r.GetN(), 617283945617);
		EXPECT_EQ(r.GetD(), 5);
	}
	{
		Rational r{1234567891234.5};
		EXPECT_EQ(r.GetN(), 2469135782469);
		EXPECT_EQ(r.GetD(), 2);
	}
	{
		Rational r{12345678912345.6};
		EXPECT_EQ(r.GetN(), 61728394561728);
		EXPECT_EQ(r.GetD(), 5);
	}
	{
		// This one overflows
		Rational r{123456789123456.7};
		EXPECT_EQ(r.GetN(), 123456789123457);
		EXPECT_EQ(r.GetD(), 1);
	}
}

TEST(Rational, operators) {
	Rational r1{1, 2};
	Rational r2{1, 4};
	auto r3 = r1 + r2;
	EXPECT_EQ(r3.GetN(), 3);
	EXPECT_EQ(r3.GetD(), 4);
}

TEST(Rational, simplify) {
	auto r = Rational{12, 24}.Simplify();
	EXPECT_EQ(r.GetN(), 1);
	EXPECT_EQ(r.GetD(), 2);
}
