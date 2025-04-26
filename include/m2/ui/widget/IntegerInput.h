#pragma once
#include "../UiWidget.h"
#include <m2/sdl/TextTexture.h>

namespace m2::widget {
	class IntegerInput : public UiWidget {
		int _value;
		sdl::TextTexture _textTexture, _plusTexture, _minusTexture;
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit IntegerInput(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction select(int value);

		// Accessors

		[[nodiscard]] int value() const { return _value; }

		// Modifiers

		void SetValue(int value);

	protected:
		UiAction OnEvent(Events& events) override;
		UiAction OnUpdate() override;
		void OnDraw() override;

	private:
		[[nodiscard]] const IntegerInputBlueprint& VariantBlueprint() const { return std::get<IntegerInputBlueprint>(blueprint->variant); }
	};
}
