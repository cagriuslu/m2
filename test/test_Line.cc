#include <gtest/gtest.h>
#include <m2/common/math/Line.h>
#include <m2/common/Math.h>

using namespace m2;

TEST(Line, construction) {
	// Line through (0,1) and (1,2): parallel is (1,1), passing through (0,1)
	auto line = Line::FromPoints(VecF{0.0f, 1.0f}, VecF{1.0f, 2.0f});
	EXPECT_EQ(line.GetParallel(), VecF(1.0f, 1.0f));

	// Perpendicular of parallel (a,b) is (-b,a)
	EXPECT_EQ(line.GetPerpendicular(), VecF(-1.0f, 1.0f));
}

TEST(Line, slopeAndIntersects) {
	// Line y = x + 1
	auto line = Line::FromPoints(VecF{0.0f, 1.0f}, VecF{1.0f, 2.0f});
	EXPECT_FLOAT_EQ(line.GetSlope(), 1.0f);
	EXPECT_FLOAT_EQ(line.GetYIntersect(), 1.0f);
	EXPECT_FLOAT_EQ(line.GetXIntersect(), -1.0f);

	// Vertical line x = 3 intersects the x-axis at x = 3
	auto vertical = Line::FromPoints(VecF{3.0f, 0.0f}, VecF{3.0f, 5.0f});
	EXPECT_FLOAT_EQ(vertical.GetXIntersect(), 3.0f);
}

TEST(Line, intersection) {
	// y = x and y = -x + 2 intersect at (1,1)
	auto line1 = Line::FromPoints(VecF{0.0f, 0.0f}, VecF{1.0f, 1.0f});
	auto line2 = Line::FromPoints(VecF{0.0f, 2.0f}, VecF{1.0f, 1.0f});
	auto intersection = line1.GetIntersectionPointWith(line2);
	ASSERT_TRUE(intersection.has_value());
	EXPECT_FLOAT_EQ(intersection->GetX(), 1.0f);
	EXPECT_FLOAT_EQ(intersection->GetY(), 1.0f);

	// Vertical line x = 3 crosses y = x at (3,3)
	auto vertical = Line::FromPoints(VecF{3.0f, 0.0f}, VecF{3.0f, 5.0f});
	auto verticalIntersection = line1.GetIntersectionPointWith(vertical);
	ASSERT_TRUE(verticalIntersection.has_value());
	EXPECT_FLOAT_EQ(verticalIntersection->GetX(), 3.0f);
	EXPECT_FLOAT_EQ(verticalIntersection->GetY(), 3.0f);

	// Parallel lines don't intersect
	auto parallel = Line::FromPoints(VecF{0.0f, 1.0f}, VecF{1.0f, 2.0f});
	EXPECT_FALSE(line1.GetIntersectionPointWith(parallel).has_value());
}

TEST(Line, angles) {
	auto horizontal = Line::FromPoints(VecF{0.0f, 0.0f}, VecF{1.0f, 0.0f});

	// Rotating the horizontal line onto a vertical line needs +90 degrees
	auto vertical = Line::FromPoints(VecF{0.0f, 0.0f}, VecF{0.0f, 1.0f});
	EXPECT_FLOAT_EQ(horizontal.GetAngleTo(vertical), PI_DIV2);

	// The smaller rotation to align onto a line at 135 degrees is -45 degrees
	auto diagonal = Line::FromPoints(VecF{0.0f, 0.0f}, VecF{-1.0f, 1.0f});
	EXPECT_FLOAT_EQ(horizontal.GetSmallerAngleTo(diagonal), -PI / 4.0f);
}
