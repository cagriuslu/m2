#pragma once
#include "../Widget.h"

namespace m2::ui {
	// Forward declaration
	struct State;
}

namespace m2::ui::widget {
	class NestedUi : public Widget {
		std::unique_ptr<State> _ui;
		int _inner_x{}, _inner_y{};

	public:
		explicit NestedUi(State* parent, const WidgetBlueprint* blueprint);
		void on_position_update(const SDL_Rect& rect_px) final;
		Action on_event(Events& events) final;
		Action on_update() final;
		void on_draw() final;
	};
}
