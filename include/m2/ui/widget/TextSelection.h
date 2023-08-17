#pragma once
#include "../Widget.h"
#include "../../sdl/Detail.hh"

namespace m2::ui::widget {
	struct TextSelection : public Widget {
		const TextSelectionBlueprint& text_selection_blueprint;

		std::vector<std::string> list;
		unsigned selection{};
		sdl::FontTexture font_texture;
		bool inc_depressed{};
		bool dec_depressed{};

		explicit TextSelection(const WidgetBlueprint* blueprint);
		Action update_content() override;
		Action handle_events(Events& events) override;
		Action select(unsigned index);
		void draw() override;
	};
}
