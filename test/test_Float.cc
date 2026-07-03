#include <gtest/gtest.h>
#include <m2/common/math/Float.h>
#include <format>

using namespace m2;

TEST(Float, basic) {
	Float f1;
	EXPECT_FLOAT_EQ(f1.ToFloat(), 0.0f);
	EXPECT_TRUE(f1.IsZero());

	Float f2{2.5f};
	EXPECT_FLOAT_EQ(f2.ToFloat(), 2.5f);

	Float f3{3};
	EXPECT_FLOAT_EQ(f3.ToFloat(), 3.0f);
	EXPECT_EQ(f3.ToInteger(), 3);

	EXPECT_FLOAT_EQ(Float::Zero().ToFloat(), 0.0f);
	EXPECT_FLOAT_EQ(Float::One().ToFloat(), 1.0f);
}

TEST(Float, operators) {
	EXPECT_EQ(Float{2.0f} + Float{3.0f}, Float{5.0f});
	EXPECT_EQ(Float{5.0f} - Float{3.0f}, Float{2.0f});
	EXPECT_EQ(Float{2.0f} * Float{3.0f}, Float{6.0f});
	EXPECT_EQ(Float{6.0f} / Float{3.0f}, Float{2.0f});
	EXPECT_EQ(-Float{2.0f}, Float{-2.0f});

	Float f = Float{1.0f};
	f += Float{2.0f};
	EXPECT_EQ(f, Float{3.0f});

	EXPECT_TRUE(Float{1.0f} < Float{2.0f});
	EXPECT_TRUE(Float{2.0f} <= Float{2.0f});
	EXPECT_TRUE(Float{2.0f} > Float{1.0f});
	EXPECT_TRUE(Float{2.0f} >= Float{2.0f});
	EXPECT_NE(Float{1.0f}, Float{2.0f});

	EXPECT_TRUE(static_cast<bool>(Float{1.0f}));
	EXPECT_FALSE(static_cast<bool>(Float{}));
}

TEST(Float, accessors) {
	EXPECT_TRUE(Float{1.0f}.IsPositive());
	EXPECT_FALSE(Float{-1.0f}.IsPositive());
	EXPECT_TRUE(Float{-1.0f}.IsNegative());
	EXPECT_FALSE(Float{1.0f}.IsNegative());

	EXPECT_TRUE(Float{1.0f}.IsEqual(Float{1.05f}, Float{0.1f}));
	EXPECT_FALSE(Float{1.0f}.IsEqual(Float{1.2f}, Float{0.1f}));
	EXPECT_TRUE(Float{1.0f}.IsLess(Float{2.0f}, Float{0.1f}));
	EXPECT_FALSE(Float{1.0f}.IsLess(Float{1.05f}, Float{0.1f}));
}

TEST(Float, modifiers) {
	EXPECT_FLOAT_EQ(Float{-2.0f}.AbsoluteValue().ToFloat(), 2.0f);
	EXPECT_FLOAT_EQ(Float{4.0f}.Inverse().ToFloat(), 0.25f);
	EXPECT_FLOAT_EQ(Float{2.0f}.Power(Float{3.0f}).ToFloat(), 8.0f);
	EXPECT_FLOAT_EQ(Float{9.0f}.SquareRoot().ToFloat(), 3.0f);
}

TEST(Float, ToString) {
	EXPECT_EQ(Float{1.5f}.ToString(), "+000001.500000");
	EXPECT_STREQ(std::format("{}", Float{-2.25f}).c_str(), "-000002.250000");
}
