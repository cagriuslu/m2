#include <m2/game/Selection.h>
#include <m2/Game.h>

using namespace m2;

std::optional<RectI> Selection::IntegerSelectionRectM() const {
	if (_positionM.first) {
		return RectI::from_corners(_positionM.first->iround(), SecondPosition().iround());
	}
	return std::nullopt;
}
std::optional<RectF> Selection::CellSelectionRectM() const {
	if (_positionM.first) {
		return RectF::from_corners(_positionM.first->round() - 0.5f, SecondPosition().round() + 0.5f);
	}
	return std::nullopt;
}
std::optional<RectF> Selection::HalfCellSelectionRectM() const {
	if (_positionM.first) {
		return RectF::from_corners(_positionM.first->hround(), SecondPosition().hround());
	}
	return std::nullopt;
}
std::optional<RectF> Selection::SelectionRectM() const {
	if (_positionM.first) {
		return RectF::from_corners(*_positionM.first, SecondPosition());
	}
	return std::nullopt;
}

VecF Selection::SecondPosition() const {
	return _positionM.second ? *_positionM.second : M2_GAME.MousePositionWorldM();
}
