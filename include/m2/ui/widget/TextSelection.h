#pragma once
#include <m2/ui/Widget.h>
#include <m2/sdl/Font.h>

namespace m2::ui::widget {
	class TextSelection : public Widget {
		std::vector<std::string> _list;
		unsigned _selection{};
		sdl::FontTexture _font_texture, _plus_texture, _minus_texture;
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit TextSelection(State* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		Action on_event(Events& events) override;
		void on_draw() override;

		// Accessors
		[[nodiscard]] inline const std::string& selection() const { return _list[_selection]; }

		// Modifiers
		void recreate();
		Action trigger_action(unsigned new_selection);

	private:
		[[nodiscard]] const TextSelectionBlueprint& text_selection_blueprint() const { return std::get<TextSelectionBlueprint>(blueprint->variant); }
	};
}
