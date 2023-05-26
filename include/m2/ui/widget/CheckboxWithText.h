#pragma once
#include "AbstractButton.h"
#include "../../sdl/Detail.hh"

namespace m2::ui::widget {
	struct CheckboxWithText : public AbstractButton {
		bool state;
		sdl::TextureUniquePtr font_texture;

		explicit CheckboxWithText(const WidgetBlueprint* blueprint);
		void draw() override;
	};
}
