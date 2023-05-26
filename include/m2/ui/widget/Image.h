#pragma once
#include "../../Sprite.h"
#include "AbstractButton.h"

namespace m2::ui::widget {
	struct Image : public AbstractButton {
		const Sprite* sprite;

		explicit Image(const WidgetBlueprint* blueprint);
		Action update_content() override;
		void draw() override;
	};
}
