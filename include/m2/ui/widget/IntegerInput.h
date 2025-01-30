#pragma once
#include "../UiWidget.h"
#include <m2/sdl/TextTexture.h>

namespace m2::widget {
	class IntegerInput : public UiWidget {
		int _value;
		sdl::TextTexture _textTexture, _plus_texture, _minus_texture;
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit IntegerInput(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction HandleEvents(Events& events) override;
		UiAction select(int value);
		UiAction UpdateContent() override;
		void Draw() override;

		// Accessors

		[[nodiscard]] int value() const { return _value; }

		// Modifiers

		void SetValue(int value);

	private:
		[[nodiscard]] const IntegerInputBlueprint& VariantBlueprint() const { return std::get<IntegerInputBlueprint>(blueprint->variant); }
	};
}
