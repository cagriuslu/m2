#pragma once
#include "../UiAction.h"
#include "../../Events.h"
#include "../UiWidget.h"
#include "../../sdl/TextTexture.h"

namespace m2::widget {
	class ImageSelection : public UiWidget {
		sdl::TextTexture _plus_texture, _minus_texture;
		unsigned _selection{};
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit ImageSelection(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction on_event(Events& events) override;
		UiAction select(unsigned index);
		void on_draw() override;

		// Modifiers
		void recreate();
		UiAction trigger_action(unsigned new_selection);

		[[nodiscard]] inline m2g::pb::SpriteType selection() const { return std::get<ImageSelectionBlueprint>(blueprint->variant).list[_selection]; }
	};
}
