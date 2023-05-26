#pragma once
#include <string>
#include <vector>
#include <functional>

namespace m2::ui::widget {
	struct TextSelectionBlueprint {
		std::vector<std::string> list;
		unsigned initial_selection{};
		std::function<Action(const std::string& selection)> action_callback;
	};
}
