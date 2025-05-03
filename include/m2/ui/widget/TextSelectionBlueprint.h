#pragma once
#include <m2/protobuf/Detail.h>
#include <string>
#include <vector>

namespace m2::widget {
	// Forward declaration
	class TextSelection;

	struct TextSelectionBlueprint {
		using ReturnValue = std::variant<std::string, int>;
		struct Option {
			std::string text;
			ReturnValue return_value;
			bool initiallySelected{};
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

		std::function<void(TextSelection& self)> onCreate{};
		std::function<void(TextSelection& self, std::optional<int> indexUnderMouse)> onHover{};
		std::function<void(TextSelection& self)> offHover{};
		std::function<UiAction(TextSelection& self)> onUpdate{};
		std::function<UiAction(TextSelection& self)> onAction{};
	};

	/// Converts a sequence of protobuf enum values into test selection options where the text is the name of the enum,
	/// and the return value is the integer value of the enum.
	template <typename PbEnumIteratorT>
	TextSelectionBlueprint::Options ToTextSelectionOptions(PbEnumIteratorT first, PbEnumIteratorT last) {
		TextSelectionBlueprint::Options options;
		for (; first != last; ++first) {
			options.emplace_back(pb::enum_name(*first), I(*first));
		}
		return options;
	}
}  // namespace m2::widget
