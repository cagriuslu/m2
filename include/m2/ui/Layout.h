#pragma once
#include <m2/ui/UiWidgetBlueprint.h>
#include <m2/Meta.h>
#include <vector>
#include <variant>

namespace m2 {
	struct FixedSpacer { int amount{1}; };
	struct DynamicSpacer { int minAmount{1}; }; /// DynamicSpacers squeezes Widget to their min size
	using VerticalLayoutItem = std::variant<UiWidgetBlueprint, FixedSpacer, DynamicSpacer>;

	/// Arranges widgets vertically. Height of the widgets are considered as minimum values. x, y, w, and h of widgets
	/// are adjusted to form a vertical layout.
	expected<std::vector<UiWidgetBlueprint>> MakeVerticalLayout(int w, int h, int minimumSpacing, const std::vector<VerticalLayoutItem>& items);
}
