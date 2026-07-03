#include <gtest/gtest.h>
#include <m2/common/math/RectE.h>

using namespace m2;

TEST(RectE, Construction) {
	RectE defaultRect;
	EXPECT_EQ(defaultRect.x, Exact{});
	EXPECT_EQ(defaultRect.y, Exact{});
	EXPECT_EQ(defaultRect.w, Exact{});
	EXPECT_EQ(defaultRect.h, Exact{});

	RectE intRect{1, 2, 3, 4};
	EXPECT_EQ(intRect.x, Exact{1});
	EXPECT_EQ(intRect.y, Exact{2});
	EXPECT_EQ(intRect.w, Exact{3});
	EXPECT_EQ(intRect.h, Exact{4});

	RectE exactRect{Exact{5}, Exact{6}, Exact{7}, Exact{8}};
	EXPECT_EQ(exactRect.w, Exact{7});
	EXPECT_EQ(exactRect.h, Exact{8});
}

TEST(RectE, Equality) {
	EXPECT_TRUE((RectE{1, 2, 3, 4} == RectE{1, 2, 3, 4}));
	EXPECT_FALSE((RectE{1, 2, 3, 4} == RectE{1, 2, 3, 5}));
}

TEST(RectE, BoolConversion) {
	// A rectangle with positive width and height is truthy
	EXPECT_TRUE(static_cast<bool>(RectE{0, 0, 2, 2}));
	// A zero-extent rectangle is falsy
	EXPECT_FALSE(static_cast<bool>(RectE{0, 0, 0, 2}));
	EXPECT_FALSE(static_cast<bool>(RectE{0, 0, 2, 0}));
	EXPECT_FALSE(static_cast<bool>(RectE{}));
}

TEST(RectE, CornersAndCenters) {
	RectE rect{1, 2, 4, 6};
	EXPECT_EQ(rect.GetX2(), Exact{5});       // x + w
	EXPECT_EQ(rect.GetY2(), Exact{8});       // y + h
	EXPECT_EQ(rect.GetXCenter(), Exact{3});  // x + w/2
	EXPECT_EQ(rect.GetYCenter(), Exact{5});  // y + h/2
}
