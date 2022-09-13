#ifndef M2_UI_BLUEPRINT_IMAGEBLUEPRINT_H
#define M2_UI_BLUEPRINT_IMAGEBLUEPRINT_H

#include "../../Sprite.h"
#include "../Action.h"
#include <SDL.h>
#include <optional>
#include <functional>

namespace m2::ui {
	struct ImageBlueprint {
		std::string initial_sprite_key;
		std::function<std::pair<Action,std::optional<SpriteKey>>(void)> update_callback;
		std::function<Action(void)> action_callback;
		SDL_Scancode kb_shortcut;
	};
}

#endif //M2_UI_BLUEPRINT_IMAGEBLUEPRINT_H
