#pragma once
#include "../Action.h"
#include "../TextAlignment.h"
#include <string>
#include <SDL.h>

namespace m2::ui::widget {
	struct TextBlueprint {
		std::string initial_text;
		TextAlignment alignment;
		bool is_toggle{}; // TODO
		std::function<std::pair<Action,std::optional<std::string>>(void)> update_callback;
		std::function<Action(void)> action_callback;
		SDL_Scancode kb_shortcut{};
	};
}
