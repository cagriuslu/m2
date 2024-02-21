#pragma once
#include "AbstractButton.h"
#include <m2/sdl/Font.h>

namespace m2::ui::widget {
	class CheckboxWithText : public AbstractButton {
		bool _state;
		sdl::FontTexture _font_texture;

	public:
		explicit CheckboxWithText(State* parent, const WidgetBlueprint* blueprint);
		void on_draw() override;

		// Accessors
		bool state() const { return _state; }

		// Modifiers
		void recreate();
		Action trigger_action(bool new_state);

		friend struct AbstractButton;
	};
}
