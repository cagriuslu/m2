#pragma once
#include "../Events.h"
#include "../math/VecF.h"

namespace m2 {
	class Selection {
		RectI _screenBoundaryPx;
		std::pair<std::optional<VecF>, std::optional<VecF>> _positionM;

	public:
		explicit Selection(const RectI& screenBoundaryPx) : _screenBoundaryPx(screenBoundaryPx) {}

		// Accessors

		[[nodiscard]] const RectI& ScreenBoundaryPx() const { return _screenBoundaryPx; }
		[[nodiscard]] bool IsComplete() const { return _positionM.first && _positionM.second; }
		[[nodiscard]] std::optional<RectI> IntegerSelectionRectM() const;
		/// Returns cell selection where the cells are centered at integer positions, and the width/height are 1 meter,
		/// which leads to corners being at -0.5 and +0.5.
		[[nodiscard]] std::optional<RectF> CellSelectionRectM() const;
		/// Returns half-cell selection where the corners are either at 0.5 steps, or integer steps.
		[[nodiscard]] std::optional<RectF> HalfCellSelectionRectM() const;
		/// Returns raw selection.
		[[nodiscard]] std::optional<RectF> SelectionRectM() const;

		// Modifiers

		void Reset() { _positionM = {}; }
		void SetFirstAndClearSecondPositionM(VecF positionM) { _positionM.first = std::move(positionM); _positionM.second.reset(); }
		void SetSecondPositionIfFirstSetM(VecF positionM) { if (_positionM.first) { _positionM.second = std::move(positionM); } }

	private:
		[[nodiscard]] VecF SecondPosition() const;
	};
}