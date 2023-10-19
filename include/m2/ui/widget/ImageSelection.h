#pragma once
#include "../Action.h"
#include "../../Events.h"
#include "../Widget.h"

namespace m2::ui::widget {
	class ImageSelection : public Widget {
		unsigned _selection{};
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit ImageSelection(State* parent, const WidgetBlueprint* blueprint);
		Action on_event(Events& events) override;
		Action select(unsigned index);
		void on_draw() override;

		inline m2g::pb::SpriteType selection() const { return std::get<ImageSelectionBlueprint>(blueprint->variant).list[_selection]; }
	};
}
