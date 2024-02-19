#pragma once
#include <string>
#include <vector>

namespace m2::ui::widget {
	// Forward declaration
	class TextListSelection;

	struct TextListSelectionBlueprint {
		using Options = std::vector<std::string>;
		Options initial_list{};
		int line_count{1};
		bool allow_multiple_selection{};
		bool show_scroll_bar{true};

		std::function<std::optional<Options>(const TextListSelection &self)> on_create{};
		std::function<std::pair<Action, std::optional<Options>>(const TextListSelection &self)> on_update{};
		std::function<Action(const TextListSelection &self)> on_action{};
	};
}  // namespace m2::ui::widget
