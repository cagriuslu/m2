#ifndef M2_IMAGE_H
#define M2_IMAGE_H

#include "AbstractButton.h"
#include <SDL.h>
#include <optional>
#include <functional>

namespace m2::ui::wdg {
	struct ImageBlueprint {
		SDL_Rect initial_texture_rect;
		std::function<std::pair<Action,std::optional<SDL_Rect>>(void)> update_callback;
		std::function<Action(void)> action_callback;
		SDL_Scancode kb_shortcut;
	};

	struct ImageState : public AbstractButtonState {
		SDL_Rect texture_rect;

        explicit ImageState(const WidgetBlueprint* blueprint);
        Action update_content() override;
        // void draw() override;
	};
}

#endif //M2_IMAGE_H
