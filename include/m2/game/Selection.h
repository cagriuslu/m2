#pragma once
#include "../Events.h"
#include "../math/VecF.h"

namespace m2 {
	class SelectionResult {
		VecF _mouse_position_m;
		std::pair<std::optional<VecF>, std::optional<VecF>> _primary_selection_position_m;
		std::pair<std::optional<VecF>, std::optional<VecF>> _secondary_selection_position_m;

	public:
		explicit SelectionResult(const Events& events);

		[[nodiscard]] inline bool is_primary_selection_finished() const { return _primary_selection_position_m.first && _primary_selection_position_m.second; }
		[[nodiscard]] inline bool is_secondary_selection_finished() const { return _secondary_selection_position_m.first && _secondary_selection_position_m.second; }

		// Returns integer selection coordinates
		[[nodiscard]] std::optional<std::pair<VecI, VecI>> primary_int_selection_position_m() const;
		[[nodiscard]] std::optional<std::pair<VecI, VecI>> secondary_int_selection_position_m() const;
		// Returns cell selection coordinates
		// For example, the center cell is x:[-0.5,0.5] y:[-0.5,0.5].
		[[nodiscard]] std::optional<std::pair<VecF, VecF>> primary_cell_selection_position_m() const;
		[[nodiscard]] std::optional<std::pair<VecF, VecF>> secondary_cell_selection_position_m() const;
		// Returns half cell selection coordinates
		// For example, x:[0,0.5] y:[0,0.5] is a cell.
		[[nodiscard]] std::optional<std::pair<VecF, VecF>> primary_halfcell_selection_position_m() const;
		[[nodiscard]] std::optional<std::pair<VecF, VecF>> secondary_halfcell_selection_position_m() const;
		// Returns selection coordinates
		[[nodiscard]] std::optional<std::pair<VecF, VecF>> primary_selection_position_m() const;
		[[nodiscard]] std::optional<std::pair<VecF, VecF>> secondary_selection_position_m() const;

	private:
		enum class RoundingType {
			CELL,
			HALF_CELL,
			CONTINUOUS
		};
		[[nodiscard]] std::optional<std::pair<VecI, VecI>> selection_position_int_m(const std::pair<std::optional<VecF>, std::optional<VecF>>& raw_selection_position_m) const;
		[[nodiscard]] std::optional<std::pair<VecF, VecF>> selection_position_m(const std::pair<std::optional<VecF>, std::optional<VecF>>& raw_selection_position_m, RoundingType rounding_type) const;
	};
}