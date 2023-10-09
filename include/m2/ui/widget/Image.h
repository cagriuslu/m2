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

	void draw_sprite(const Sprite& sprite, const SDL_Rect& dst_rect);
}
