#pragma once
#include <string>
#include <sstream>
#include <functional>

namespace m2::widget {
	// Forward declaration
	class TextInput;

	struct TextInputBlueprint {
		std::string initial_text;
		std::function<std::pair<UiAction,std::optional<std::string>>(const TextInput& self)> on_action{};
	};
}
