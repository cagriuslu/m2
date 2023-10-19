#pragma once
#include <functional>
#include <optional>

namespace m2::ui::widget {
	// Forward declaration
	class IntegerSelection;

	struct IntegerSelectionBlueprint {
		int min_value{}, max_value{}; /// Values are inclusive
		int initial_value{};

		std::function<std::optional<int>(const IntegerSelection& self)> on_update;
		std::function<Action(const IntegerSelection& self)> on_action;
	};
}
