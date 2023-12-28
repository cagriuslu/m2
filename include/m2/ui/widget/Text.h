#pragma once
#include "AbstractButton.h"
#include <m2/sdl/Font.h>

namespace m2::ui::widget {
	class Text : public AbstractButton {
		sdl::FontTexture _font_texture;

	public:
		explicit Text(State* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		void on_draw() override;

	private:
		[[nodiscard]] const TextBlueprint& text_blueprint() const { return std::get<TextBlueprint>(blueprint->variant); }
	};
}
