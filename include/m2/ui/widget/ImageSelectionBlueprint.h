#pragma once
#include "../Action.h"
#include <SpriteType.pb.h>
#include <vector>

namespace m2::ui::widget {
	struct ImageSelectionBlueprint {
		std::vector<m2g::pb::SpriteType> list;
		unsigned initial_selection{};
		std::function<Action(m2g::pb::SpriteType selection)> action_callback;
	};
}
