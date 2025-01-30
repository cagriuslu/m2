#pragma once
#include "../UiAction.h"
#include <string>
#include <functional>
#include <SDL.h>

namespace m2::widget {
	// Forward declaration
	class CheckboxWithText;

	struct CheckboxWithTextBlueprint {
		std::string text;
		bool initial_state{};
		SDL_Scancode kb_shortcut{};

		std::function<void(CheckboxWithText& self)> onCreate{};
		std::function<UiAction(CheckboxWithText& self)> onAction{};
	};
}
