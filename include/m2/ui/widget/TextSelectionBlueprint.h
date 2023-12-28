#pragma once

#include <string>
#include <vector>
#include <functional>

namespace m2::ui::widget {
	// Forward declaration
	class TextSelection;

	struct TextSelectionBlueprint {
		using Options = std::vector<std::string>;
		Options initial_list;

		std::function<std::optional<Options>(const TextSelection &self)> on_create{};
		std::function<std::pair<Action, std::optional<Options>>(const TextSelection &self)> on_update{};
		std::function<Action(const TextSelection &self)> on_action{};
	};
}
