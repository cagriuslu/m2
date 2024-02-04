#pragma once
#include "../Widget.h"
#include <m2/sdl/Font.h>

namespace m2::ui::widget {
	class IntegerInput : public Widget {
		sdl::FontTexture _font_texture, _plus_texture, _minus_texture;
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit IntegerInput(State* parent, const WidgetBlueprint* blueprint);
		Action on_event(Events& events) override;
		Action select(int value);
		Action on_update() override;
		void on_draw() override;

		[[nodiscard]] int value() const { return _font_texture.int_value(); }

	private:
		[[nodiscard]] const IntegerInputBlueprint& integer_selection_blueprint() const { return std::get<IntegerInputBlueprint>(blueprint->variant); }
	};
}
