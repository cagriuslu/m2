#pragma once
#include "../Widget.h"

namespace m2::ui {
	// Forward declaration
	struct State;
}

namespace m2::ui::widget {
	struct NestedUi : public Widget {
		std::unique_ptr<State> ui;
		int inner_x{}, inner_y{};

		explicit NestedUi(const WidgetBlueprint* blueprint);
		void update_position(const SDL_Rect& rect_px) final;
		Action handle_events(Events& events) final;
		Action update_content() final;
		void draw() final;
	};
}
