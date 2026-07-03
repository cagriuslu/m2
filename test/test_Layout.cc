#include <gtest/gtest.h>
#include <m2/ui/Layout.h>
#include <vector>

using namespace m2;

static UiWidgetBlueprint WidgetOfHeight(const int height) {
	UiWidgetBlueprint widget;
	widget.h = height;
	return widget;
}

TEST(Layout, SingleWidgetFillsAvailableSpace) {
	// w=100, h=100, spacing=10, one widget of min height 20.
	// requiredHeight = 20 (top+bottom padding) + 20 (widget) = 40.
	// The single widget absorbs all 60 units of extra space.
	std::vector<VerticalLayoutItem> items;
	items.emplace_back(WidgetOfHeight(20));

	const auto result = MakeVerticalLayout(100, 100, 10, items);
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(result->size(), 1u);

	const auto& widget = result->front();
	EXPECT_EQ(widget.x, 10);            // left padding == spacing
	EXPECT_EQ(widget.y, 10);            // top padding == spacing
	EXPECT_EQ(widget.w, 80);            // w - 2 * spacing
	EXPECT_EQ(widget.h, 80);            // 20 + all 60 extra units
}

TEST(Layout, TwoWidgetsSplitSpaceRemainderToLast) {
	// w=100, h=101, spacing=10, two widgets each of min height 10.
	// requiredHeight = 20 + 10 + 10 (inter-widget spacing) + 10 = 50.
	// extra = 51, /2 = 25 per widget; the trailing 1 unit goes to the last widget.
	std::vector<VerticalLayoutItem> items;
	items.emplace_back(WidgetOfHeight(10));
	items.emplace_back(WidgetOfHeight(10));

	const auto result = MakeVerticalLayout(100, 101, 10, items);
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(result->size(), 2u);

	const auto& first = (*result)[0];
	EXPECT_EQ(first.x, 10);
	EXPECT_EQ(first.y, 10);
	EXPECT_EQ(first.w, 80);
	EXPECT_EQ(first.h, 35);             // 10 + 25

	const auto& second = (*result)[1];
	EXPECT_EQ(second.x, 10);
	EXPECT_EQ(second.y, 55);            // 10 + 35 + 10 spacing
	EXPECT_EQ(second.w, 80);
	EXPECT_EQ(second.h, 36);            // 10 + 25 + 1 remainder
}

TEST(Layout, DynamicSpacerKeepsWidgetAtMinHeight) {
	// With a dynamic spacer present, widgets stay at their min height and the
	// spacer absorbs the extra space.
	std::vector<VerticalLayoutItem> items;
	items.emplace_back(WidgetOfHeight(20));
	items.emplace_back(DynamicSpacer{5});

	const auto result = MakeVerticalLayout(100, 100, 10, items);
	ASSERT_TRUE(result.has_value());
	ASSERT_EQ(result->size(), 1u);     // only the widget appears in the output

	const auto& widget = result->front();
	EXPECT_EQ(widget.x, 10);
	EXPECT_EQ(widget.y, 10);
	EXPECT_EQ(widget.w, 80);
	EXPECT_EQ(widget.h, 20);           // stays at min, not expanded
}

TEST(Layout, OverfullHeightReturnsError) {
	// A widget taller than the available height can't fit.
	std::vector<VerticalLayoutItem> items;
	items.emplace_back(WidgetOfHeight(50));

	const auto result = MakeVerticalLayout(100, 30, 10, items);
	EXPECT_FALSE(result.has_value());
}

TEST(Layout, InsufficientWidthReturnsError) {
	// w must exceed 2 * spacing to leave room for content.
	std::vector<VerticalLayoutItem> items;
	items.emplace_back(WidgetOfHeight(5));

	const auto result = MakeVerticalLayout(20, 100, 10, items);
	EXPECT_FALSE(result.has_value());
}
