#pragma once
#include <m2/sdl/TextTexture.h>
#include <m2/ui/UiWidget.h>

namespace m2::widget {
	class TextSelection : public UiWidget {
		struct Option {
			TextSelectionBlueprint::Option blueprint_option;
			bool is_selected{};
			std::optional<sdl::TextTextureAndDestination> text_texture_and_destination;
		};
		std::vector<Option> _options;

		// Applicable to +/- selection mode
		std::optional<sdl::TextTextureAndDestination> _plus_texture, _minus_texture;
		bool _plus_depressed{}, _minus_depressed{};

		// Applicable to scrollable list
		std::optional<sdl::TextTextureAndDestination> _up_arrow_texture, _down_arrow_texture;
		int _top_index{};

	public:
		explicit TextSelection(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction on_update() override;
		UiAction on_event(Events& events) override;
		void on_draw() override;

		// Accessors

		[[nodiscard]] std::vector<TextSelectionBlueprint::ReturnValue> selections() const;

		// Modifiers

		void set_options(const TextSelectionBlueprint::Options& options);
		void set_options(TextSelectionBlueprint::Options&& options);
		void set_unique_selection(int index);

	protected:
		void on_resize(const RectI& oldRect, const RectI& newRect) override;

	private:
		[[nodiscard]] const TextSelectionBlueprint& text_list_selection_blueprint() const;
		UiAction increment_selection();
		UiAction decrement_selection();
	};
}
