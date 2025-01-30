#pragma once
#include "../UiWidget.h"
#include <m2/sdl/TextTexture.h>
#include <sstream>

namespace m2::widget {
	class TextInput : public UiWidget {
		std::stringstream _text_input;
		/// During initialization, the destination cannot yet be determined.
		std::optional<sdl::TextTextureAndDestination> _text_texture_and_destination_cache;

	public:
		explicit TextInput(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction HandleEvents(Events& events) override;
		void HandleFocusChange() override;
		void Draw() override;

		// Accessors
		std::string text_input() const { return _text_input.str(); }

		// Modifiers

		UiAction trigger_action(std::string new_value);

	private:
		[[nodiscard]] const TextInputBlueprint& VariantBlueprint() const { return std::get<TextInputBlueprint>(blueprint->variant); }
	};
}
