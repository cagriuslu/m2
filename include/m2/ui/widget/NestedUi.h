#pragma once
#include "../Widget.h"

namespace m2::ui {
	// Forward declaration
	struct Panel;
}

namespace m2::ui::widget {
	class NestedUi : public Widget {
		std::unique_ptr<Panel> _ui;
		int _inner_x{}, _inner_y{};

	public:
		explicit NestedUi(Panel* parent, const WidgetBlueprint* blueprint);
		void on_position_update(const RectI& rect_px) final;
		Action on_event(Events& events) final;
		Action on_update() final;
		void on_draw() final;
	};
}
