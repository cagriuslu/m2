#pragma once
#include "AbstractButton.h"
#include "../../sdl/Detail.hh"

namespace m2::ui::widget {
	struct Text : public AbstractButton {
		sdl::TextureUniquePtr font_texture;

		explicit Text(const WidgetBlueprint* blueprint);
		Action update_content() override;
		void draw() override;
	};
}
