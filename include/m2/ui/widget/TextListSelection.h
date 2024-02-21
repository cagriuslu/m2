#pragma once
#include <m2/sdl/Font.h>
#include <m2/ui/Widget.h>

namespace m2::ui::widget {
	class TextListSelection : public Widget {
		std::vector<std::pair<sdl::FontTexture, bool>> _list;
		int _top_index{};
		sdl::FontTexture _up_arrow_texture, _down_arrow_texture;

	   public:
		explicit TextListSelection(State* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		Action on_event(Events& events) override;
		void on_draw() override;

		// Accessors
		std::vector<std::string> selection() const;

		// Modifiers
		void recreate();
		Action trigger_action(/* TODO */);

	   private:
		void prepare_list(const std::vector<std::string>& entries);
		[[nodiscard]] const TextListSelectionBlueprint& text_list_selection_blueprint() const {
			return std::get<TextListSelectionBlueprint>(blueprint->variant);
		}
	};
}  // namespace m2::ui::widget
