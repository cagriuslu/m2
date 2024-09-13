#pragma once
#include "widget/CheckboxWithTextBlueprint.h"
#include "widget/ImageBlueprint.h"
#include "widget/HiddenBlueprint.h"
#include "widget/ImageSelectionBlueprint.h"
#include "widget/IntegerInputBlueprint.h"
#include "widget/ProgressBarBlueprint.h"
#include "widget/TextBlueprint.h"
#include "widget/TextInputBlueprint.h"
#include "widget/TextSelectionBlueprint.h"
#include <SDL.h>
#include <variant>

namespace m2::ui {
	struct WidgetBlueprint {
		std::string name{};
		bool initially_enabled{true};
		bool initially_focused{false};
		int x{}, y{}, w{1}, h{1}; // unitless // TODO convert to float

		// Unitless border width. If non-zero, the drawn border is at least 1 pixel. Border is drawn *inside* the Rect
		// of the widget, thus it behaves like a padding at the same time.
		float border_width{0.001f};
		// Unitless padding width. Padding is *inside* the border.
		float padding_width{0.0f};

		SDL_Color background_color{};

		using Variant = std::variant<
				widget::HiddenBlueprint,
				widget::ImageBlueprint,
				widget::TextBlueprint,
				widget::ProgressBarBlueprint,
				widget::TextInputBlueprint,
				widget::ImageSelectionBlueprint,
				widget::TextSelectionBlueprint,
				widget::IntegerInputBlueprint,
				widget::CheckboxWithTextBlueprint>;
		Variant variant;
	};
}
