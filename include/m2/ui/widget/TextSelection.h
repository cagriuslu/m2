#pragma once
#include "../Widget.h"
#include "../../sdl/Detail.hh"

namespace m2::ui::widget {
	class TextSelection : public Widget {
		std::vector<std::string> _list;
		unsigned _selection{};
		sdl::FontTexture _font_texture;
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit TextSelection(State* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		Action on_event(Events& events) override;
		Action select(unsigned index);
		void on_draw() override;

		const TextSelectionBlueprint& text_selection_blueprint() const { return std::get<TextSelectionBlueprint>(blueprint->variant); }
		inline const std::string& selection() const { return _list[_selection]; }
	};
}
