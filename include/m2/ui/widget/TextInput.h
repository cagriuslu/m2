#pragma once
#include "../Widget.h"
#include <m2/sdl/Font.h>
#include <sstream>

namespace m2::ui::widget {
	class TextInput : public Widget {
		std::stringstream _text_input;
		sdl::FontTexture _font_texture;

	public:
		explicit TextInput(State* parent, const WidgetBlueprint* blueprint);
		Action on_event(Events& events) override;
		void on_focus_change() override;
		Action on_update() override;
		void on_draw() override;

		// Accessors
		std::string text_input() const { return _text_input.str(); }

		// Modifiers
		void recreate();
		Action trigger_action(std::string new_value);
	};
}
