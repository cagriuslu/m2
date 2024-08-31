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
		float font_size_in_units{}; // Height of one line of text in UI units. '0' fills the widget.
		TextHorizontalAlignment horizontal_alignment{};
		TextVerticalAlignment vertical_alignment{};
		bool word_wrap{}; // If true, font_size_in_units must be non-zero
		RGB color{255, 255, 255};
		SDL_Scancode kb_shortcut{};

		std::function<void(Text& self)> on_create{};
		std::function<Action(Text& self)> on_update{};
		std::function<Action(const Text& self)> on_action{};
	};
}  // namespace m2::ui::widget
