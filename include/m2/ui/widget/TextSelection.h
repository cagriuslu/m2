#pragma once
#include <m2/sdl/FontTexture.h>
#include <m2/ui/Widget.h>

namespace m2::ui::widget {
	class TextSelection : public Widget {
		struct Option {
			TextSelectionBlueprint::Option blueprint_option;
			bool is_selected{};
			std::optional<sdl::FontTextureAndDestination> font_texture_and_destination;
		};
		std::vector<Option> _options;

		// Applicable to +/- selection mode
		std::optional<sdl::FontTextureAndDestination> _plus_texture, _minus_texture;
		bool _plus_depressed{}, _minus_depressed{};

		// Applicable to scrollable list
		std::optional<sdl::FontTextureAndDestination> _up_arrow_texture, _down_arrow_texture;
		int _top_index{};

	public:
		explicit TextSelection(Panel* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		Action on_event(Events& events) override;
		void on_draw() override;

		// Accessors

		[[nodiscard]] std::vector<TextSelectionBlueprint::ReturnValue> selections() const;

		// Modifiers

		void set_options(const TextSelectionBlueprint::Options& options);
		void set_options(TextSelectionBlueprint::Options&& options);
		void set_unique_selection(int index);

	protected:
		void on_resize() override;

	private:
		[[nodiscard]] const TextSelectionBlueprint& text_list_selection_blueprint() const;
		Action increment_selection();
		Action decrement_selection();
	};
}
