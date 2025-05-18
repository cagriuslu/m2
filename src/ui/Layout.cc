#include <m2/ui/Layout.h>

m2::expected<std::vector<m2::UiWidgetBlueprint>> m2::MakeVerticalLayout(const int w, const int h, const int minimumSpacing, const std::vector<VerticalLayoutItem>& items) {
	if (w <= 2 * minimumSpacing) {
		return make_unexpected("Vertical layout items don't have enough width");
	}

	std::vector<size_t> widgetIndexes;
	std::vector<size_t> dynamicSpacerIndexes;

	// Check if layout is possible, keep track of widgets and dynamic spacers
	int requiredHeight = minimumSpacing * 2; // Add top and bottom padding
	for (size_t i = 0; i < items.size(); ++i) {
		const auto& item = items[i];
		if (std::holds_alternative<UiWidgetBlueprint>(item)) {
			widgetIndexes.emplace_back(i);
			requiredHeight += std::get<UiWidgetBlueprint>(item).h; // Height of the widgets is considered the min size

			// Look ahead, add minimum spacing if necessary
			if (i + 1 < items.size() && std::holds_alternative<UiWidgetBlueprint>(items[i + 1])) {
				requiredHeight += minimumSpacing;
			}
		} else if (std::holds_alternative<FixedSpacer>(item)) {
			requiredHeight += std::get<FixedSpacer>(item).amount;
		} else if (std::holds_alternative<DynamicSpacer>(item)) {
			dynamicSpacerIndexes.emplace_back(i);
			requiredHeight += std::get<DynamicSpacer>(item).minAmount;
		}
	}
	if (h < requiredHeight) {
		return make_unexpected("Vertical layout items don't fit inside given height");
	}

	std::vector<UiWidgetBlueprint> widgets;
	if (dynamicSpacerIndexes.empty()) {
		auto totalExtraSpace = h - requiredHeight;
		const auto extraSpacePerWidget = totalExtraSpace / widgetIndexes.size();
		// Integer division might have a remainder, thus do successive subtractions

		int currentHeight = minimumSpacing; // Add top padding
		for (size_t i = 0; i < items.size(); ++i) {
			const auto& item = items[i];

			if (std::holds_alternative<UiWidgetBlueprint>(item)) {
				// Copy widget
				widgets.emplace_back(std::get<UiWidgetBlueprint>(item));
				// Adjust widget
				widgets.back().x = minimumSpacing;
				widgets.back().y = currentHeight;
				widgets.back().w = w - 2 * minimumSpacing;
				if (i == items.size() - 1) {
					// If last widget, add remained extra space
					widgets.back().h = std::get<UiWidgetBlueprint>(item).h + totalExtraSpace;
				} else {
					widgets.back().h = std::get<UiWidgetBlueprint>(item).h + extraSpacePerWidget;
				}
				// Deduct used extra space from total
				totalExtraSpace -= extraSpacePerWidget;
				// Advance current height
				currentHeight += widgets.back().h;

				// Look ahead, add minimum spacing if necessary
				if (i + 1 < items.size() && std::holds_alternative<UiWidgetBlueprint>(items[i + 1])) {
					currentHeight += minimumSpacing;
				}
			} else if (std::holds_alternative<FixedSpacer>(item)) {
				// Advance current height
				currentHeight += std::get<FixedSpacer>(item).amount;
			}
		}
	} else {
		// If there's at least one dynamic spacer, widgets are pushed to their min size
		auto totalExtraSpace = h - requiredHeight;
		const auto extraSpacePerSpacer = totalExtraSpace / dynamicSpacerIndexes.size();
		// Integer division might have a remainder, thus do successive subtractions

		int currentHeight = minimumSpacing; // Add top padding
		for (size_t i = 0; i < items.size(); ++i) {
			const auto& item = items[i];

			if (std::holds_alternative<UiWidgetBlueprint>(item)) {
				// Copy widget
				widgets.emplace_back(std::get<UiWidgetBlueprint>(item));
				// Adjust widget
				widgets.back().x = minimumSpacing;
				widgets.back().y = currentHeight;
				widgets.back().w = w - 2 * minimumSpacing;
				widgets.back().h = std::get<UiWidgetBlueprint>(item).h;
				// Advance current height
				currentHeight += widgets.back().h;

				// Look ahead, add minimum spacing if necessary
				if (i + 1 < items.size() && std::holds_alternative<UiWidgetBlueprint>(items[i + 1])) {
					currentHeight += minimumSpacing;
				}
			} else if (std::holds_alternative<FixedSpacer>(item)) {
				// Advance current height
				currentHeight += std::get<FixedSpacer>(item).amount;
			} else if (std::holds_alternative<DynamicSpacer>(item)) {
				// Advance current height
				if (i == items.size() - 1) {
					// If last spacer, add remained extra space
					currentHeight += std::get<DynamicSpacer>(item).minAmount + totalExtraSpace;
				} else {
					currentHeight += std::get<DynamicSpacer>(item).minAmount + extraSpacePerSpacer;
				}
				// Deduct used extra space from total
				totalExtraSpace -= extraSpacePerSpacer;
			}
		}
	}
	return widgets;
}