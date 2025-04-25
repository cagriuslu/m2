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

		// Accessors

		[[nodiscard]] const std::vector<Option>& GetOptions() const { return _options; }
		[[nodiscard]] std::vector<TextSelectionBlueprint::ReturnValue> selections() const;
		[[nodiscard]] std::vector<int> SelectedIndexes() const;

		// Modifiers

		void set_options(const TextSelectionBlueprint::Options& options);
		void set_options(TextSelectionBlueprint::Options&& options);
		void set_unique_selection(int index);

	protected:
		void OnResize(const RectI& oldRect, const RectI& newRect) override;
		UiAction OnEvent(Events& events) override;
		UiAction OnUpdate() override;
		void OnDraw() override;

	private:
		[[nodiscard]] const TextSelectionBlueprint& VariantBlueprint() const { return std::get<TextSelectionBlueprint>(blueprint->variant); }
		UiAction increment_selection();
		UiAction decrement_selection();
	};
}
