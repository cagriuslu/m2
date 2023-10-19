#pragma once
#include "../Action.h"
#include <m2g_SpriteType.pb.h>
#include <vector>

namespace m2::ui::widget {
	// Forward declaration
	class ImageSelection;

	struct ImageSelectionBlueprint {
		std::vector<m2g::pb::SpriteType> list;

		std::function<Action(const ImageSelection& self)> on_action;
	};
}
