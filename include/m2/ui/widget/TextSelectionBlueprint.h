#pragma once
#include <string>
#include <vector>
#include <functional>

namespace m2::ui::widget {
	struct TextSelectionBlueprint {
		using Options = std::vector<std::string>;

		Options initial_list;
		std::function<std::optional<Options>()> on_create;
		std::function<std::pair<Action,std::optional<Options>>(const Options& current_list, unsigned current_selection)> update_callback;
		std::function<Action(unsigned selection_idx, const std::string& selection)> action_callback;
	};
}
