#pragma once
#include "AbstractButton.h"
#include <m2/sdl/FontTexture.h>

namespace m2::ui::widget {
	class Text : public AbstractButton {
		std::string _current_text;
		/// Instead of generating colored font texture, generate white text and color the text before rendering.
		RGB _current_color;
		/// During initialization, the destination cannot yet be determined.
		std::optional<sdl::FontTextureAndDestination> _font_texture_and_destination_cache;

	public:
		explicit Text(Panel* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		void on_draw() override;

		// Accessors
		[[nodiscard]] std::string_view text() const { return _current_text; }
		[[nodiscard]] const RGB& color() const { return _current_color; }

		// Modifiers
		void set_text(const std::string&);
		void set_color(RGB&& c) { _current_color = c; }

	protected:
		void on_resize() override { _font_texture_and_destination_cache = std::nullopt; }

	private:
		[[nodiscard]] const TextBlueprint& text_blueprint() const { return std::get<TextBlueprint>(blueprint->variant); }
	};
}
