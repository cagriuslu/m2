#pragma once
#include <m2/sdl/FontTexture.h>
#include <m2/ui/Widget.h>

namespace m2::ui::widget {
	class TextSelection : public Widget {
		TextSelectionBlueprint::Options _list;
		std::vector<sdl::FontTexture> _option_texts;
		std::vector<bool> _selections;

		// Applicable to +/- selection mode
		sdl::FontTexture _plus_texture, _minus_texture;
		bool _plus_depressed{}, _minus_depressed{};

		// Applicable to scrollable list
		int _top_index{};
		sdl::FontTexture _up_arrow_texture, _down_arrow_texture;

	public:
		explicit TextSelection(Panel* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		Action on_event(Events& events) override;
		void on_draw() override;

		// Accessors
		[[nodiscard]] std::vector<TextSelectionBlueprint::ValueVariant> selections() const;

		// Modifiers
		void reset();
		void set_options(TextSelectionBlueprint::Options options);
		void set_selection(int index);

	private:
		[[nodiscard]] const TextSelectionBlueprint& text_list_selection_blueprint() const;
		Action increment_selection(int count);
		Action decrement_selection(int count);
	};
}
