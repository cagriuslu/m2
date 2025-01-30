#pragma once
#include "../UiAction.h"
#include <m2g_SpriteType.pb.h>
#include <SDL.h>

namespace m2::widget {
	// Forward declaration
	class Image;

	struct ImageBlueprint {
		m2g::pb::SpriteType initial_sprite{};
		SDL_Scancode kb_shortcut{};

		std::function<void(Image& self)> onCreate{};
		std::function<std::pair<UiAction,std::optional<m2g::pb::SpriteType>>(const Image& self)> onUpdate{};
		std::function<UiAction(const Image& self)> onAction{};
	};
}
