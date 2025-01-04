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
		UiAction on_event(Events& events) override;
		UiAction select(int value);
		UiAction on_update() override;
		void on_draw() override;

		// Accessors
		[[nodiscard]] int value() const { return _value; }

		// Modifiers
		void recreate();
		UiAction trigger_action(int new_value);

	private:
		[[nodiscard]] const IntegerInputBlueprint& integer_selection_blueprint() const { return std::get<IntegerInputBlueprint>(blueprint->variant); }
	};
}
