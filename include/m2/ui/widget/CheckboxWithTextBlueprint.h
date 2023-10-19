#pragma once
#include "../Action.h"
#include <string>
#include <functional>
#include <SDL.h>

namespace m2::ui::widget {
	// Forward declaration
	class CheckboxWithText;

	struct CheckboxWithTextBlueprint {
		std::string text;
		bool initial_state{};
		SDL_Scancode kb_shortcut{};

		std::function<Action(const CheckboxWithText& self)> on_action;
	};
}
