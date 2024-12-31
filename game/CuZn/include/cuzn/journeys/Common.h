#pragma once
#include <m2/game/Fsm.h>
#include <m2/math/VecF.h>
#include <m2/ui/PanelBlueprint.h>
#include <list>

std::list<m2::ui::Panel>::iterator AddCancelButton();

// Used by various Journey state machines to carry mouse click position, or cancel button press.
class PositionOrCancelSignal : public m2::FsmSignalBase {
	std::optional<m2::VecF> _world_position;
	bool _cancel{};

public:
	using m2::FsmSignalBase::FsmSignalBase;
	static PositionOrCancelSignal create_mouse_click_signal(m2::VecF world_position);
	static PositionOrCancelSignal create_cancel_signal(bool cancelled = true);

	// Accessors
	[[nodiscard]] const m2::VecF* world_position() const { return _world_position ? &*_world_position : nullptr; }
	[[nodiscard]] bool cancel() const { return _cancel; }
};
