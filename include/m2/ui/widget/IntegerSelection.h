#pragma once
#include "../UiWidget.h"
#include <m2/sdl/TextTexture.h>

namespace m2::widget {
	class IntegerSelection : public UiWidget {
		int _value;
		sdl::TextTextureAndDestination _textTexture, _plusTexture, _minusTexture;
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit IntegerSelection(UiPanel* parent, const UiWidgetBlueprint* blueprint);

		// Accessors

		[[nodiscard]] int value() const { return _value; }

		// Modifiers

		UiAction SetValue(int value);

	protected:
		void OnResize(const RectI&, const RectI&) override;
		UiAction OnEvent(Events& events) override;
		UiAction OnUpdate() override;
		void OnDraw() override;

	private:
		[[nodiscard]] const IntegerSelectionBlueprint& VariantBlueprint() const { return std::get<IntegerSelectionBlueprint>(blueprint->variant); }

		[[nodiscard]] RectI CalculateValueDrawArea() const;
		[[nodiscard]] std::pair<RectI,RectI> CalculatePlusAndMinusButtonDrawArea() const;
	};
}
