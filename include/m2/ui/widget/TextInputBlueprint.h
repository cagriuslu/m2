#pragma once
#include <string>
#include <sstream>
#include <functional>

namespace m2::widget {
	// Forward declaration
	class TextInput;

	struct TextInputBlueprint {
		std::string initial_text;

		std::function<void(TextInput& self)> onCreate{};
		std::function<std::pair<UiAction,std::optional<std::string>>(TextInput& self)> onAction{};
	};
}
