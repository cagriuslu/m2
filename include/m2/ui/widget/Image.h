#ifndef M2_IMAGE_H
#define M2_IMAGE_H

#include "AbstractButton.h"
#include "../../Sprite.h"
#include <SDL.h>
#include <optional>
#include <functional>
#include <m2/SpriteBlueprint.h>

namespace m2::ui::wdg {
	struct ImageBlueprint {
		std::string initial_sprite_key;
		std::function<std::pair<Action,std::optional<SpriteKey>>(void)> update_callback;
		std::function<Action(void)> action_callback;
		SDL_Scancode kb_shortcut;
	};

	struct ImageState : public AbstractButtonState {
		const Sprite* sprite;

        explicit ImageState(const WidgetBlueprint* blueprint);
        Action update_content() override;
        void draw() override;
	};
}

#endif //M2_IMAGE_H
