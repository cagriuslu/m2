#pragma once
#include "../Action.h"
#include <m2g_SpriteType.pb.h>
#include <vector>

namespace m2::ui::widget {
	struct ImageSelectionBlueprint {
		std::vector<m2g::pb::SpriteType> list;
		std::function<Action(m2g::pb::SpriteType selection)> action_callback;
	};
}
