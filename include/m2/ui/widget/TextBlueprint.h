#pragma once
#include <SDL.h>

#include <string>
#include <variant>

#include "../UiAction.h"
#include "../Detail.h"
#include "../../video/Color.h"

namespace m2::widget {
	// Forward declaration
	class Text;

	struct TextBlueprint {
		std::string text{};

		TextHorizontalAlignment horizontal_alignment{};

		TextVerticalAlignment vertical_alignment{};

		// If non-zero, word wrapping is enabled, and font size is fixed to the given number of units.
		// Otherwise, the drawable area is filled with the text.
		float wrapped_font_size_in_units{};

		RGB color{255, 255, 255};

		SDL_Scancode kb_shortcut{};

		std::function<void(Text& self)> onCreate{};
		std::function<UiAction(Text& self)> onUpdate{};
		std::function<UiAction(const Text& self)> onAction{};
	};
}  // namespace m2::widget
