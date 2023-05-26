#pragma once
#include "../Action.h"
#include "../../Events.h"
#include "../Widget.h"

namespace m2::ui::widget {
	struct ImageSelection : public Widget {
		unsigned selection;
		bool inc_depressed{};
		bool dec_depressed{};

		explicit ImageSelection(const WidgetBlueprint* blueprint);
		Action handle_events(Events& events) override;
		Action select(unsigned index);
		void draw() override;
	};
}
