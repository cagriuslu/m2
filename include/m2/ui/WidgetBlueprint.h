#pragma once
#include "widget/CheckboxWithTextBlueprint.h"
#include "widget/ImageBlueprint.h"
#include "widget/HiddenBlueprint.h"
#include "widget/ImageSelectionBlueprint.h"
#include "widget/IntegerInputBlueprint.h"
#include "widget/NestedUiBlueprint.h"
#include "widget/ProgressBarBlueprint.h"
#include "widget/TextBlueprint.h"
#include "widget/TextInputBlueprint.h"
#include "widget/TextSelectionBlueprint.h"
#include <SDL.h>
#include <variant>

namespace m2::ui {
	struct WidgetBlueprint {
		bool initially_enabled{true};
		bool initially_focused{false};
		int x{}, y{}, w{1}, h{1}; // unitless
		unsigned border_width_px{1};
		unsigned padding_width_px{};
		SDL_Color background_color{};

		using Variant = std::variant<
				widget::NestedUiBlueprint,
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
