#pragma once
#include <string>
#include <vector>
#include <functional>

namespace m2::ui::widget {
	struct TextSelectionBlueprint {
		std::vector<std::string> initial_list;
		unsigned initial_selection{};
		std::function<std::pair<Action,std::optional<std::vector<std::string>>>(void)> update_callback;
		std::function<Action(const std::string& selection)> action_callback;
	};
}
