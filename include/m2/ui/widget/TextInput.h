#pragma once
#include "../Widget.h"
#include "../../sdl/Detail.hh"
#include <sstream>

namespace m2::ui::widget {
	struct TextInput : public Widget {
		std::stringstream text_input;
		sdl::TextureUniquePtr font_texture;
		std::string font_texture_str;

		explicit TextInput(const WidgetBlueprint* blueprint);
		~TextInput() override;
		Action handle_events(Events& events) override;
		Action update_content() override;
		void draw() override;
	};
}