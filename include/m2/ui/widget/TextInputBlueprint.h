#pragma once
#include <string>
#include <sstream>
#include <functional>

namespace m2::ui::widget {
	struct TextInputBlueprint {
		std::string initial_text;
		std::function<Action(std::stringstream&)> action_callback;
	};
}
