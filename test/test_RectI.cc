#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <m2/math/RectI.h>
#include <m2/math/RectF.h>

using namespace m2;

TEST(RectI, from_intersecting_cells) {
	EXPECT_EQ(RectI::CreateFromIntersectingCells(RectF{1.5f, 1.5f, 2.0f, 2.0f}), (RectI{1, 1, 3, 3}));
}
