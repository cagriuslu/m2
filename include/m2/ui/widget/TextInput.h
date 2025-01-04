#pragma once
#include "../UiWidget.h"
#include <m2/sdl/TextTexture.h>
#include <sstream>

namespace m2::widget {
	class TextInput : public UiWidget {
		std::stringstream _text_input;
		sdl::TextTexture _textTexture;

	public:
		explicit TextInput(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction on_event(Events& events) override;
		void on_focus_change() override;
		UiAction on_update() override;
		void on_draw() override;

		// Accessors
		std::string text_input() const { return _text_input.str(); }

		// Modifiers
		void recreate();
		UiAction trigger_action(std::string new_value);
	};
}
