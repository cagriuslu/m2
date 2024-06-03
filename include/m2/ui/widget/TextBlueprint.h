#pragma once
#include <SDL.h>

#include <string>
#include <variant>

#include "../Action.h"
#include "../TextAlignment.h"
#include "../../Color.h"

namespace m2::ui::widget {
	// Forward declaration
	class Text;

	struct TextBlueprint {
		std::string text{};
		float font_size{};  // unitless, '0' -> fill the widget
		TextAlignment alignment{};
		RGB color{255, 255, 255};
		bool is_toggle{};  // TODO
		SDL_Scancode kb_shortcut{};

		std::function<void(Text& self)> on_create{};
		std::function<Action(Text& self)> on_update{};
		std::function<Action(const Text& self)> on_action{};
	};
}  // namespace m2::ui::widget
