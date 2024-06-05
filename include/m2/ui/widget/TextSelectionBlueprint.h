#pragma once
#include <string>
#include <vector>

namespace m2::ui::widget {
	// Forward declaration
	class TextSelection;

	struct TextSelectionBlueprint {
		using ValueVariant = std::variant<std::string, int>;
		using Option = std::pair<std::string, ValueVariant>; // <DisplayName, Value>
		using Options = std::vector<Option>;
		Options initial_list{};

		// 0: Selection with +/- buttons
		// 1: Dropdown
		// 2: Scrollable list
		int line_count{};

		// Applicable only to scrollable list
		bool allow_multiple_selection{};
		bool show_scroll_bar{true};

		std::function<void(TextSelection& self)> on_create{};
		std::function<Action(TextSelection& self)> on_update{};
		std::function<Action(TextSelection& self)> on_action{};
	};
}  // namespace m2::ui::widget
