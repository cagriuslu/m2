#pragma once
#include <m2/ui/Blueprint.h>

namespace cuzn {
	/// @returns If true, the Blueprint will be used to select a card and return it from sync UI execution.
	///          If false, the Blueprint will be used as custom UI dialog, and will not return a selection.
	m2::ui::Blueprint generate_cards_window(bool return_selection);
}
