#pragma once
#include "../Action.h"
#include <m2g_SpriteType.pb.h>
#include <SDL.h>

namespace m2::ui::widget {
	struct ImageBlueprint {
		m2g::pb::SpriteType initial_sprite{};
		std::function<std::pair<Action,std::optional<m2g::pb::SpriteType>>(void)> update_callback;
		std::function<Action(void)> action_callback;
		SDL_Scancode kb_shortcut{};
	};
}
