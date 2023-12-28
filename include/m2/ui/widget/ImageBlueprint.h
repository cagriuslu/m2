#pragma once
#include "../Action.h"
#include <m2g_SpriteType.pb.h>
#include <SDL.h>

namespace m2::ui::widget {
	// Forward declaration
	class Image;

	struct ImageBlueprint {
		m2g::pb::SpriteType initial_sprite{};
		SDL_Scancode kb_shortcut{};

		std::function<std::pair<Action,std::optional<m2g::pb::SpriteType>>(const Image& self)> on_update{};
		std::function<Action(const Image& self)> on_action{};
	};
}
