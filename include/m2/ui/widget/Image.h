#pragma once
#include <m2/video/Sprite.h>
#include "AbstractButton.h"

namespace m2::widget {
	class Image : public AbstractButton {
		m2g::pb::SpriteType _spriteType{};

	public:
		explicit Image(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction UpdateContent() override;
		void Draw() override;

		// Modifiers

		void SetSpriteType(m2g::pb::SpriteType);

	private:
		[[nodiscard]] const ImageBlueprint& VariantBlueprint() const { return std::get<ImageBlueprint>(blueprint->variant); }
	};
}
