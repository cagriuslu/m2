#pragma once
#include "AbstractButton.h"
#include <m2/sdl/FontTexture.h>

namespace m2::ui::widget {
	class Text : public AbstractButton {
		std::string _string;
		sdl::FontTexture _font_texture;
		// Instead of generating colored font textures, generate white text and color the text before rendering.
		RGB _color_override;

	public:
		explicit Text(State* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		void on_draw() override;

		// Accessors
		[[nodiscard]] std::string_view text() const { return _string; }
		[[nodiscard]] const RGB& color() const { return _color_override; }

		// Modifiers
		void set_text(const std::string&);
		void set_color(RGB&&);

	private:
		[[nodiscard]] const TextBlueprint& text_blueprint() const { return std::get<TextBlueprint>(blueprint->variant); }
		[[nodiscard]] int widget_width_in_chars() const;
		[[nodiscard]] float widget_letter_width_in_pixels() const;
	};
}
