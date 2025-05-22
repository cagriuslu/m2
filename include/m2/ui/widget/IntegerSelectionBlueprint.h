#pragma once
#include <functional>
#include <optional>

namespace m2::widget {
	// Forward declaration
	class IntegerSelection;

	struct IntegerSelectionBlueprint {
		int min_value{}, max_value{}; /// Values are inclusive
		int initial_value{};

		std::function<std::optional<int>(const IntegerSelection& self)> onCreate{};
		std::function<std::optional<int>(const IntegerSelection& self)> onUpdate{};
		std::function<UiAction(const IntegerSelection& self)> onAction{};
	};
}
