#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <m2/RectI.h>
#include <m2/RectF.h>

using namespace m2;

TEST(RectI, from_intersecting_cells) {
	EXPECT_EQ(RectI::from_intersecting_cells(RectF{1.5f, 1.5f, 2.0f, 2.0f}), (RectI{1, 1, 3, 3}));
}