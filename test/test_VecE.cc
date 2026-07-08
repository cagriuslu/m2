#include <gtest/gtest.h>
#include <m2/common/math/VecE.h>
#include <m2/common/math/VecF.h>
#include <cmath>
#include <format>

using namespace m2;

TEST(VecE, basic) {
	VecE v1;
	EXPECT_EQ(v1.GetX(), Exact{});
	EXPECT_EQ(v1.GetY(), Exact{});

	VecE v2{3, 4};
	EXPECT_EQ(v2.GetX(), Exact{3});
	EXPECT_EQ(v2.GetY(), Exact{4});

	VecE v3{Exact{1}, Exact{2}};
	EXPECT_EQ(v3.GetX(), Exact{1});
	EXPECT_EQ(v3.GetY(), Exact{2});
}

TEST(VecE, operators) {
	EXPECT_EQ(VecE(1, 2) + VecE(3, 4), VecE(4, 6));
	EXPECT_EQ(VecE(3, 4) - VecE(1, 2), VecE(2, 2));
	EXPECT_EQ(VecE(1, 2) - Exact{1}, VecE(0, 1));
	EXPECT_EQ(-VecE(1, 2), VecE(-1, -2));
	EXPECT_EQ(VecE(1, 2) * Exact{2}, VecE(2, 4));
	EXPECT_EQ(VecE(4, 6) / Exact{2}, VecE(2, 3));

	VecE v{1, 2};
	v += VecE(3, 4);
	EXPECT_EQ(v, VecE(4, 6));
	v -= VecE(1, 1);
	EXPECT_EQ(v, VecE(3, 5));

	EXPECT_TRUE(VecE(1, 2) == VecE(1, 2));
	EXPECT_FALSE(VecE(1, 2) == VecE(2, 1));
}

TEST(VecE, lengths) {
	VecE v{3, 4};
	EXPECT_EQ(v.GetLengthSquaredFE(), Exact{25});
	EXPECT_NEAR(v.GetLengthFE().ToFloat(), 5.0f, 0.01f);

	VecE a{1, 1}, b{4, 4};
	EXPECT_EQ(a.GetDistanceToSquaredFE(b), Exact{18});
	EXPECT_NEAR(a.GetDistanceToFE(b).ToFloat(), sqrtf(18.0f), 0.01f);
}

TEST(VecE, Normalize) {
	EXPECT_EQ(VecE{}.Normalize(), VecE{});
	EXPECT_NEAR(VecE(3, 4).Normalize().GetLengthFE().ToFloat(), 1.0f, 0.01f);
}

TEST(VecE, VecFConversion) {
	auto f = static_cast<VecF>(VecE(3, 4));
	EXPECT_FLOAT_EQ(f.GetX(), 3.0f);
	EXPECT_FLOAT_EQ(f.GetY(), 4.0f);
}

TEST(VecE, Hash) {
	VecEHash hasher;
	EXPECT_EQ(hasher(VecE(1, 2)), hasher(VecE(1, 2)));
	EXPECT_NE(hasher(VecE(1, 2)), hasher(VecE(3, 4)));
}

TEST(VecE, ToString) {
	const auto expected = std::format("{{x:{},y:{}}}", Exact{1}, Exact{2});
	EXPECT_EQ(std::format("{}", VecE(1, 2)), expected);
}
