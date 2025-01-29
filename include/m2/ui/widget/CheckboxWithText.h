#pragma once
#include "AbstractButton.h"
#include <m2/sdl/TextTexture.h>

namespace m2::widget {
	class CheckboxWithText : public AbstractButton {
		bool _state;
		sdl::TextTexture _textTexture;

	public:
		explicit CheckboxWithText(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		void on_draw() override;

		// Accessors
		bool state() const { return _state; }

		// Modifiers
		void recreate();

		friend struct AbstractButton;
	};
}
