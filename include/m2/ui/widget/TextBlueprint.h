#pragma once
#include <SDL.h>

#include <string>
#include <variant>

#include "../Action.h"
#include "../TextAlignment.h"

namespace m2::ui::widget {
	// Forward declaration
	class Text;

	struct TextBlueprint {
		std::string initial_text{};
		float font_size{};  // unitless, '0' -> fill the widget
		TextAlignment alignment{};
		bool is_toggle{};  // TODO
		SDL_Scancode kb_shortcut{};

		std::function<std::optional<std::string>(const Text& self)> on_create{};
		std::function<std::pair<Action, std::optional<std::string>>(const Text& self)> on_update{};
		std::function<Action(const Text& self)> on_action{};
	};
}  // namespace m2::ui::widget
