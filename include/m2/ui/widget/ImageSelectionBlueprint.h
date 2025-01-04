#pragma once
#include "../UiAction.h"
#include <m2g_SpriteType.pb.h>
#include <vector>

namespace m2::widget {
	// Forward declaration
	class ImageSelection;

	struct ImageSelectionBlueprint {
		std::vector<m2g::pb::SpriteType> list;

		std::function<UiAction(const ImageSelection& self)> on_action{};
	};
}
