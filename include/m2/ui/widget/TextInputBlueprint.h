#pragma once
#include <string>
#include <sstream>
#include <functional>

namespace m2::ui::widget {
	// Forward declaration
	class TextInput;

	struct TextInputBlueprint {
		std::string initial_text;
		std::function<std::pair<Action,std::optional<std::string>>(const TextInput& self)> on_action;
	};
}
