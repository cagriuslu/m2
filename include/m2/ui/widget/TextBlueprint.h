#pragma once
#include <SDL.h>

#include <string>
#include <variant>

#include "../Action.h"
#include "../Detail.h"
#include "../../Color.h"

namespace m2::ui::widget {
	// Forward declaration
	class Text;

	struct TextBlueprint {
		std::string text{};

		TextHorizontalAlignment horizontal_alignment{};

		TextVerticalAlignment vertical_alignment{};

		// If non-zero, word wrapping is enabled, and font size is fixed to the given number of units.
		float wrapped_font_size_in_units{};

		RGB color{255, 255, 255};

		SDL_Scancode kb_shortcut{};

		std::function<void(Text& self)> on_create{};
		std::function<Action(Text& self)> on_update{};
		std::function<Action(const Text& self)> on_action{};
	};
}  // namespace m2::ui::widget
