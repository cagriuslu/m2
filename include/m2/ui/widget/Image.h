#pragma once
#include "../../Sprite.h"
#include "AbstractButton.h"

namespace m2::ui::widget {
	class Image : public AbstractButton {
		const Sprite* _sprite;

	public:
		explicit Image(State* parent, const WidgetBlueprint* blueprint);
		Action on_update() override;
		void on_draw() override;
	};

	void draw_sprite(const Sprite& sprite, const SDL_Rect& dst_rect);
}
