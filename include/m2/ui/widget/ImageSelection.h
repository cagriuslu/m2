#pragma once
#include "../Action.h"
#include "../../Events.h"
#include "../Widget.h"
#include "../../sdl/FontTexture.h"

namespace m2::ui::widget {
	class ImageSelection : public Widget {
		sdl::FontTexture _plus_texture, _minus_texture;
		unsigned _selection{};
		bool _inc_depressed{};
		bool _dec_depressed{};

	public:
		explicit ImageSelection(Panel* parent, const WidgetBlueprint* blueprint);
		Action on_event(Events& events) override;
		Action select(unsigned index);
		void on_draw() override;

		// Modifiers
		void recreate();
		Action trigger_action(unsigned new_selection);

		[[nodiscard]] inline m2g::pb::SpriteType selection() const { return std::get<ImageSelectionBlueprint>(blueprint->variant).list[_selection]; }
	};
}
