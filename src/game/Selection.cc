#include <m2/game/Selection.h>
#include <m2/Game.h>
#include <m2/Log.h>

using namespace m2;

Selection::Selection(const RectI& screenBoundaryPx) : _screenBoundaryPx(screenBoundaryPx) {
	LOG_DEBUG("Enabling selection");
}
Selection::~Selection() {
	LOG_DEBUG("Disabling selection");
}

std::optional<std::pair<VecI,VecI>> Selection::IntegerSelectionsM() const {
	if (_positionM.first) {
		return std::make_pair(_positionM.first->iround(), SecondPosition().iround());
	}
	return std::nullopt;
}
std::optional<RectI> Selection::IntegerSelectionRectM() const {
	if (_positionM.first) {
		return RectI::from_corners(_positionM.first->iround(), SecondPosition().iround());
	}
	return std::nullopt;
}

std::optional<std::pair<VecF,VecF>> Selection::CellSelectionsM() const {
	if (_positionM.first) {
		return std::make_pair(_positionM.first->round() - 0.5f, SecondPosition().round() + 0.5f);
	}
	return std::nullopt;
}
std::optional<RectF> Selection::CellSelectionRectM() const {
	if (_positionM.first) {
		return RectF::from_corners(_positionM.first->round() - 0.5f, SecondPosition().round() + 0.5f);
	}
	return std::nullopt;
}

std::optional<std::pair<VecF,VecF>> Selection::HalfCellSelectionsM() const {
	if (_positionM.first) {
		return std::make_pair(_positionM.first->hround(), SecondPosition().hround());
	}
	return std::nullopt;
}
std::optional<RectF> Selection::HalfCellSelectionRectM() const {
	if (_positionM.first) {
		return RectF::from_corners(_positionM.first->hround(), SecondPosition().hround());
	}
	return std::nullopt;
}

std::optional<std::pair<VecF,VecF>> Selection::SelectionsM() const {
	if (_positionM.first) {
		return std::make_pair(*_positionM.first, SecondPosition());
	}
	return std::nullopt;
}
std::optional<RectF> Selection::SelectionRectM() const {
	if (_positionM.first) {
		return RectF::from_corners(*_positionM.first, SecondPosition());
	}
	return std::nullopt;
}

void Selection::Reset() {
	LOG_DEBUG("Resetting selection");
	_positionM = {};
}
void Selection::SetFirstAndClearSecondPositionM(VecF positionM) {
	LOG_DEBUG("Storing first point of selection");
	_positionM.first = std::move(positionM);
	_positionM.second.reset();
}
void Selection::SetSecondPositionIfFirstSetM(VecF positionM) {
	if (_positionM.first) {
		LOG_DEBUG("Storing second point of selection");
		_positionM.second = std::move(positionM);
	} else {
		// Not an error, selection could be reset before the mouse button is lifted.
	}
}

VecF Selection::SecondPosition() const {
	return _positionM.second ? *_positionM.second : M2_GAME.MousePositionWorldM();
}
