#pragma once
#include "AbstractButton.h"
#include <m2/sdl/TextTexture.h>

namespace m2::widget {
	class Text : public AbstractButton {
		std::string _current_text;
		/// Instead of generating colored font texture, generate white text and color the text before rendering.
		RGB _current_color;
		/// During initialization, the destination cannot yet be determined.
		std::optional<sdl::TextTextureAndDestination> _text_texture_and_destination_cache;

	public:
		explicit Text(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction UpdateContent() override;
		void Draw() override;

		// Accessors
		[[nodiscard]] std::string_view text() const { return _current_text; }
		[[nodiscard]] const RGB& color() const { return _current_color; }

		// Modifiers
		void set_text(const std::string&);
		void set_color(RGB&& c) { _current_color = c; }

	protected:
		void HandleResize(const RectI& oldRect, const RectI& newRect) override;

	private:
		[[nodiscard]] const TextBlueprint& VariantBlueprint() const { return std::get<TextBlueprint>(blueprint->variant); }
	};
}
