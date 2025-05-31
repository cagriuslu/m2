#pragma once
#include "../Events.h"
#include "../math/VecF.h"

namespace m2 {
	// Base class of all Selection states
	class SelectionStateBase {
	public:
		virtual ~SelectionStateBase() = default;
	};

	class Selection {
		RectI _screenBoundaryPx;
		std::pair<std::optional<VecF>, std::optional<VecF>> _positionM;

	public:
		/// Pointer to the state. State can be used to store information about the selection, ex. an object held afloat.
		std::unique_ptr<SelectionStateBase> state{};

		explicit Selection(const RectI& screenBoundaryPx);
		~Selection();

		// Accessors

		[[nodiscard]] const RectI& ScreenBoundaryPx() const { return _screenBoundaryPx; }
		/// Signifies that the selection is completed by lifting of the mouse button
		[[nodiscard]] bool IsComplete() const { return _positionM.first && _positionM.second; }

		[[nodiscard]] std::optional<std::pair<VecI,VecI>> IntegerSelectionsM() const;
		[[nodiscard]] std::optional<RectI> IntegerSelectionRectM() const;

		/// Returns cell selection where the cells are centered at integer positions, and the width/height are 1 meter,
		/// which leads to corners being at -0.5 and +0.5.
		[[nodiscard]] std::optional<std::pair<VecF,VecF>> CellSelectionsM() const;
		[[nodiscard]] std::optional<RectF> CellSelectionRectM() const;

		/// Returns half-cell selection where the corners are either at 0.5 steps, or integer steps.
		[[nodiscard]] std::optional<std::pair<VecF,VecF>> HalfCellSelectionsM() const;
		[[nodiscard]] std::optional<RectF> HalfCellSelectionRectM() const;

		/// Returns raw selection.
		[[nodiscard]] std::optional<std::pair<VecF,VecF>> SelectionsM() const;
		[[nodiscard]] std::optional<RectF> SelectionRectM() const;

		// Modifiers

		void Reset();
		void SetFirstAndClearSecondPositionM(VecF positionM);
		void SetSecondPositionIfFirstSetM(VecF positionM);

	private:
		[[nodiscard]] VecF SecondPosition() const;
	};
}