#pragma once
#include <string>
#include <vector>

namespace m2::ui::widget {
	// Forward declaration
	class TextSelection;

	struct TextSelectionBlueprint {
		using ReturnValue = std::variant<std::string, int>;
		struct Option {
			std::string text;
			ReturnValue return_value;
			RGB text_color{255, 255, 255};
		};
		using Options = std::vector<Option>;
		/// Convenience type that can sort options based on the text
		static constexpr auto OptionsSorter = [](const Option& a, const Option& b) { return a.text < b.text; };

		/// Options
		Options options{};

		/// 0: Selection with +/- buttons
		/// 1: Dropdown
		/// 1+: Scrollable list
		int line_count{};

		/// Applicable only to scrollable list
		bool allow_multiple_selection{};
		bool show_scroll_bar{true};

		std::function<void(TextSelection& self)> on_create{};
		std::function<Action(TextSelection& self)> on_update{};
		std::function<Action(TextSelection& self)> on_action{};
	};
}  // namespace m2::ui::widget
