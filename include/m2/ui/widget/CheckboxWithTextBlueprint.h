#pragma once
#include "../Action.h"
#include <string>
#include <functional>
#include <SDL.h>

namespace m2::ui::widget {
	struct CheckboxWithTextBlueprint {
		std::string text;
		bool initial_state{};
		std::function<Action(bool state)> action_callback;
		SDL_Scancode kb_shortcut{};
	};
}
