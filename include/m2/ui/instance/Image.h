#ifndef M2_UI_INSTANCE_IMAGE_H
#define M2_UI_INSTANCE_IMAGE_H

#include "../widget/AbstractButton.h"
#include "../../Sprite.h"
#include <SDL.h>
#include <optional>
#include <functional>
#include <m2/SpriteBlueprint.h>

namespace m2::ui {
	struct ImageState : public wdg::AbstractButtonState {
		const Sprite* sprite;

        explicit ImageState(const WidgetBlueprint* blueprint);
        Action update_content() override;
        void draw() override;
	};
}

#endif //M2_UI_INSTANCE_IMAGE_H
