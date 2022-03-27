#ifndef M2_NESTEDUI_H
#define M2_NESTEDUI_H

namespace m2::ui {
	struct Blueprint;
	struct State;
}

namespace m2::ui::element {
	struct NestedUIBlueprint {
		const Blueprint& ui;
	};

	struct NestedUIState {
		State& ui;
	};
}

#endif //M2_NESTEDUI_H
