#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <m2/RectF.h>
#include <m2/VecI.h>

using namespace m2;

TEST(RectF, intersecting_cells) {
	ASSERT_THAT((RectF{0.0f, 0.0f, 0.5f, 0.5f}).intersecting_cells(), testing::ElementsAre(
			VecI{0, 0}));

	ASSERT_THAT((RectF{0.0f, 0.0f, 1.0f, 1.0f}).intersecting_cells(), testing::ElementsAre(
			VecI{0, 0}));

	ASSERT_THAT((RectF{0.0f, 0.0f, 1.5f, 1.0f}).intersecting_cells(), testing::ElementsAre(
			VecI{0, 0}, VecI{1, 0}));

	ASSERT_THAT((RectF{0.0f, 0.0f, 1.5f, 1.5f}).intersecting_cells(), testing::ElementsAre(
			VecI{0, 0}, VecI{1, 0}, VecI{0, 1}, VecI{1, 1}));

	ASSERT_THAT((RectF{0.5f, 0.5f, 1.25f, 1.25f}).intersecting_cells(), testing::ElementsAre(
			VecI{0, 0}, VecI{1, 0}, VecI{0, 1}, VecI{1, 1}));

	ASSERT_THAT((RectF{-0.5f, -0.5f, 1.0f, 1.0f}).intersecting_cells(), testing::ElementsAre(
			VecI{-1, -1}, VecI{0, -1}, VecI{-1, 0}, VecI{0, 0}));

	ASSERT_THAT((RectF{-2.0f, -2.0f, 4.0f, 4.0f}).intersecting_cells(), testing::ElementsAre(
			VecI{-2, -2}, VecI{-1, -2}, VecI{0, -2}, VecI{1, -2},
			VecI{-2, -1}, VecI{-1, -1}, VecI{0, -1}, VecI{1, -1},
			VecI{-2, 0}, VecI{-1, 0}, VecI{0, 0}, VecI{1, 0},
			VecI{-2, 1}, VecI{-1, 1}, VecI{0, 1}, VecI{1, 1}));

	ASSERT_THAT((RectF{-2.0f, -2.0f, 4.1f, 4.1f}).intersecting_cells(), testing::ElementsAre(
			VecI{-2, -2}, VecI{-1, -2}, VecI{0, -2}, VecI{1, -2}, VecI{2, -2},
			VecI{-2, -1}, VecI{-1, -1}, VecI{0, -1}, VecI{1, -1}, VecI{2, -1},
			VecI{-2, 0}, VecI{-1, 0}, VecI{0, 0}, VecI{1, 0}, VecI{2, 0},
			VecI{-2, 1}, VecI{-1, 1}, VecI{0, 1}, VecI{1, 1}, VecI{2, 1},
			VecI{-2, 2}, VecI{-1, 2}, VecI{0, 2}, VecI{1, 2}, VecI{2, 2}));
}
