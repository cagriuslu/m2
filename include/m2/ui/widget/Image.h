#pragma once
#include <m2/video/Sprite.h>
#include "AbstractButton.h"

namespace m2::widget {
	class Image : public AbstractButton {
		m2g::pb::SpriteType _spriteType{};

	public:
		explicit Image(UiPanel* parent, const UiWidgetBlueprint* blueprint);
		UiAction on_update() override;
		void on_draw() override;

		void set_sprite(m2g::pb::SpriteType);

	private:
		[[nodiscard]] const ImageBlueprint& image_blueprint() const { return std::get<ImageBlueprint>(blueprint->variant); }
	};
}
