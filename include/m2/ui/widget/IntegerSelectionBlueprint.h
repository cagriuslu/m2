#pragma once
#include <functional>
#include <optional>

namespace m2::ui::widget {
	struct IntegerSelectionBlueprint {
		int min_value{}, max_value{}; /// Values are inclusive
		int initial_value{};
		std::function<std::optional<int>(void)> update_callback;
		std::function<Action(int value)> action_callback;
	};
}
