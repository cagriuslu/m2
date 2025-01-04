#pragma once
#include <functional>
#include <optional>

namespace m2::widget {
	// Forward declaration
	class IntegerInput;

	struct IntegerInputBlueprint {
		int min_value{}, max_value{}; /// Values are inclusive
		int initial_value{};

		std::function<std::optional<int>(const IntegerInput& self)> on_create{};
		std::function<std::optional<int>(const IntegerInput& self)> on_update{};
		std::function<UiAction(const IntegerInput& self)> on_action{};
	};
}
