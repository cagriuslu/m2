#pragma once
#include "../../Sprite.h"
#include "AbstractButton.h"

namespace m2::ui::widget {
	class Image : public AbstractButton {
		std::pair<m2g::pb::SpriteType, const Sprite*> _sprite{};

	public:
		explicit Image(Panel* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		void on_draw() override;

		void set_sprite(m2g::pb::SpriteType);

	private:
		[[nodiscard]] const ImageBlueprint& image_blueprint() const { return std::get<ImageBlueprint>(blueprint->variant); }
	};
}
