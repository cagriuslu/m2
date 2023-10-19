#pragma once
#include "AbstractButton.h"
#include "../../sdl/Detail.hh"

namespace m2::ui::widget {
	class Text : public AbstractButton {
		sdl::TextureUniquePtr _font_texture;

	public:
		explicit Text(State* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		void on_draw() override;
	};
}
