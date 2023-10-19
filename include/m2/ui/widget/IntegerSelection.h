#pragma once
#include "../Widget.h"
#include "../../sdl/Detail.hh"

namespace m2::ui::widget {
	class IntegerSelection : public Widget {
		int _value;
		sdl::TextureUniquePtr _font_texture;
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit IntegerSelection(State* parent, const WidgetBlueprint* blueprint);
		Action on_event(Events& events) override;
		Action select(int value);
		Action on_update() override;
		void on_draw() override;

		inline int value() const { return _value; }
	};
}
