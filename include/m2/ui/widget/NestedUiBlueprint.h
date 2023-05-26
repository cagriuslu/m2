#pragma once

// Forward declaration
namespace m2::ui {
	struct Blueprint;
}

namespace m2::ui::widget {
	struct NestedUiBlueprint {
		const Blueprint* ui;
	};
}
