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
		std::optional<sdl::TextTextureAndDestination> _plusTexture, _minusTexture;
		bool _plusDepressed{}, _minusDepressed{};

		// Applicable to scrollable list
		std::optional<sdl::TextTextureAndDestination> _upArrowTexture, _downArrowTexture;
		int _topIndex{};
		std::optional<int> _hoveredIndex;

	public:
		explicit TextSelection(UiPanel* parent, const UiWidgetBlueprint* blueprint);

		// Accessors

		[[nodiscard]] const std::vector<Option>& GetOptions() const { return _options; }
		[[nodiscard]] const Option& GetOption(const int i) const { return _options[i]; }
		[[nodiscard]] std::vector<TextSelectionBlueprint::ReturnValue> GetSelectedOptions() const;
		[[nodiscard]] std::optional<int> GetIndexOfFirstSelection() const;
		[[nodiscard]] std::vector<int> GetSelectedIndexes() const;

		// Modifiers

		void SetOptions(const TextSelectionBlueprint::Options& options);
		void SetOptions(TextSelectionBlueprint::Options&& options);
		[[nodiscard]] Option& GetOption(const int i) { return _options[i]; }
		void SetUniqueSelectionIndex(int index);

	protected:
		void OnResize(const RectI& oldRect, const RectI& newRect) override;
		void OnHover() override;
		void OffHover() override;
		UiAction OnEvent(Events& events) override;
		UiAction OnUpdate() override;
		void OnDraw() override;

	private:
		[[nodiscard]] const TextSelectionBlueprint& VariantBlueprint() const { return std::get<TextSelectionBlueprint>(blueprint->variant); }
		/// Returns the coordinates of the whole text area.
		[[nodiscard]] RectI GetTextRects() const;
		/// Returns the coordinates of the text area of the given row. Row is ignored if not scrollable list.
		[[nodiscard]] RectI GetTextRectOfRow(int row = 0) const;
		/// Returns option-index-and-text-rect pair of the given row. Row is ignored if not scrollable list.
		[[nodiscard]] std::optional<std::pair<int,RectI>> GetOptionIndexAndTextRectOfRow(int row = 0) const;
		void RenewHoverIfNecessary();
		UiAction IncrementSelection();
		UiAction DecrementSelection();
	};
}
