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
		UiAction select(unsigned index);

		// Accessors

		[[nodiscard]] m2g::pb::SpriteType selection() const { return VariantBlueprint().list[_selection]; }

	protected:
		UiAction OnEvent(Events& events) override;
		void OnDraw() override;

	private:
		[[nodiscard]] const ImageSelectionBlueprint& VariantBlueprint() const { return std::get<ImageSelectionBlueprint>(blueprint->variant); }
	};
}
