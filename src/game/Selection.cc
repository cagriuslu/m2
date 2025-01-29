#include <m2/game/Selection.h>
#include <m2/Game.h>

using namespace m2;

SelectionResult::SelectionResult(const Events &events) : _mouse_position_m(M2_GAME.MousePositionWorldM()), _primary_selection_position_m(events.primary_selection_position_m()), _secondary_selection_position_m(events.secondary_selection_position_m()) {}

std::optional<std::pair<VecI, VecI>> SelectionResult::primary_int_selection_position_m() const {
	return selection_position_int_m(_primary_selection_position_m);
}
std::optional<RectI> SelectionResult::PrimaryIntegerRoundedSelectionRectM() const {
	if (const auto positions = primary_int_selection_position_m()) {
		return RectI::from_corners(positions->first, positions->second);
	}
	return std::nullopt;
}
std::optional<std::pair<VecF, VecF>> SelectionResult::primary_cell_selection_position_m() const {
	return selection_position_m(_primary_selection_position_m, RoundingType::CELL);
}
std::optional<std::pair<VecF, VecF>> SelectionResult::primary_halfcell_selection_position_m() const {
	return selection_position_m(_primary_selection_position_m, RoundingType::HALF_CELL);
}
std::optional<std::pair<VecF, VecF>> SelectionResult::primary_selection_position_m() const {
	return selection_position_m(_primary_selection_position_m, RoundingType::CONTINUOUS);
}
std::optional<std::pair<VecI, VecI>> SelectionResult::secondary_int_selection_position_m() const {
	return selection_position_int_m(_secondary_selection_position_m);
}
std::optional<RectI> SelectionResult::SecondaryIntegerRoundedSelectionRectM() const {
	if (const auto positions = secondary_int_selection_position_m()) {
		return RectI{positions->first.x, positions->first.y, positions->second.x - positions->first.x + 1, positions->second.y - positions->first.y + 1};
	}
	return std::nullopt;
}
std::optional<std::pair<VecF, VecF>> SelectionResult::secondary_cell_selection_position_m() const {
	return selection_position_m(_secondary_selection_position_m, RoundingType::CELL);
}
std::optional<std::pair<VecF, VecF>> SelectionResult::secondary_halfcell_selection_position_m() const {
	return selection_position_m(_secondary_selection_position_m, RoundingType::HALF_CELL);
}
std::optional<std::pair<VecF, VecF>> SelectionResult::secondary_selection_position_m() const {
	return selection_position_m(_secondary_selection_position_m, RoundingType::CONTINUOUS);
}

std::optional<std::pair<VecI, VecI>> SelectionResult::selection_position_int_m(const std::pair<std::optional<VecF>, std::optional<VecF>>& raw_selection_position_m) const {
	if (raw_selection_position_m.first) {
		// Decide what the second position is, mouse position or the second selection position
		auto second_position = raw_selection_position_m.second ? *raw_selection_position_m.second : _mouse_position_m;
		// Find top-left and bottom-right corners
		auto rect = RectF::from_corners(*raw_selection_position_m.first, second_position);
		return std::make_pair(rect.top_left().iround(), rect.bottom_right().iround());
	}
	return std::nullopt;
}
std::optional<std::pair<VecF, VecF>> SelectionResult::selection_position_m(const std::pair<std::optional<VecF>, std::optional<VecF>>& raw_selection_position_m, RoundingType rounding_type) const {
	auto round = [&rounding_type](const VecF& position) -> VecF {
		if (rounding_type == RoundingType::CELL) {
			return position.round();
		} else if (rounding_type == RoundingType::HALF_CELL) {
			return position.hround();
		} else if (rounding_type == RoundingType::CONTINUOUS) {
			return position;
		} else {
			throw M2_ERROR("Implementation error");
		}
	};

	if (raw_selection_position_m.first) {
		// Decide what the second position is, mouse position or the second selection position
		auto second_position = raw_selection_position_m.second ? *raw_selection_position_m.second : _mouse_position_m;
		// Find top-left and bottom-right corners
		auto rect = RectF::from_corners(*raw_selection_position_m.first, second_position);
		if (rounding_type == RoundingType::CELL) {
			return std::make_pair(round(rect.top_left()) - VecF{0.5f, 0.5f}, round(rect.bottom_right()) + VecF{0.5f, 0.5f});
		} else {
			return std::make_pair(round(rect.top_left()), round(rect.bottom_right()));
		}
	}
	return std::nullopt;
}
