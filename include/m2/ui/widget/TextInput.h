#pragma once
#include "../Widget.h"
#include "../../sdl/Detail.hh"
#include <sstream>

namespace m2::ui::widget {
	class TextInput : public Widget {
		std::stringstream _text_input;
		sdl::TextureUniquePtr _font_texture;
		std::string _font_texture_str;

	public:
		explicit TextInput(State* parent, const WidgetBlueprint* blueprint);
		Action on_event(Events& events) override;
		void on_focus_change() override;
		Action on_update() override;
		void on_draw() override;

		inline std::string text_input() const { return _text_input.str(); }
	};
}
