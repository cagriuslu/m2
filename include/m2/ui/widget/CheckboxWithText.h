#pragma once
#include "AbstractButton.h"
#include <m2/sdl/TextTexture.h>

namespace m2::widget {
	class CheckboxWithText : public AbstractButton {
		bool _state;
		sdl::TextTexture _textTexture;

	public:
		explicit CheckboxWithText(UiPanel* parent, const UiWidgetBlueprint* blueprint);

		// Accessors

		[[nodiscard]] bool GetState() const { return _state; }

	protected:
		void OnDraw() override;

	private:
		[[nodiscard]] const CheckboxWithTextBlueprint& VariantBlueprint() const { return std::get<CheckboxWithTextBlueprint>(blueprint->variant); }

		friend struct AbstractButton;
	};
}
