#pragma once
#include "../Widget.h"
#include "../../sdl/Detail.hh"

namespace m2::ui::widget {
	struct IntegerSelection : public Widget {
		int value;
		sdl::TextureUniquePtr font_texture;
		bool inc_depressed{};
		bool dec_depressed{};

		explicit IntegerSelection(const WidgetBlueprint* blueprint);
		Action handle_events(Events& events) override;
		Action select(int value);
		Action update_content() override;
		void draw() override;
	};
}
