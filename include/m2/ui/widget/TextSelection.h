#pragma once
#include "../Widget.h"
#include "../../sdl/Detail.hh"

namespace m2::ui::widget {
	struct TextSelection : public Widget {
		unsigned selection;
		sdl::TextureUniquePtr font_texture;
		bool inc_depressed{};
		bool dec_depressed{};

		explicit TextSelection(const WidgetBlueprint* blueprint);
		Action handle_events(Events& events) override;
		Action select(unsigned index);
		void draw() override;
	};
}
