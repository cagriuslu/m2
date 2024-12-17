#pragma once
#include "../Widget.h"
#include <m2/sdl/FontTexture.h>

namespace m2::ui::widget {
	class IntegerInput : public Widget {
		int _value;
		sdl::FontTexture _fontTexture, _plus_texture, _minus_texture;
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit IntegerInput(Panel* parent, const WidgetBlueprint* blueprint);
		Action on_event(Events& events) override;
		Action select(int value);
		Action on_update() override;
		void on_draw() override;

		// Accessors
		[[nodiscard]] int value() const { return _value; }

		// Modifiers
		void recreate();
		Action trigger_action(int new_value);

	private:
		[[nodiscard]] const IntegerInputBlueprint& integer_selection_blueprint() const { return std::get<IntegerInputBlueprint>(blueprint->variant); }
	};
}
