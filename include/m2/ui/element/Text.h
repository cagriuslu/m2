#ifndef M2_TEXT_H
#define M2_TEXT_H

#include "AbstractButton.h"
#include <SDL.h>
#include <functional>
#include <optional>
#include <string>

namespace m2::ui::element {
	struct TextBlueprint {
		std::string_view initial_text;
		std::function<std::pair<Action,std::optional<std::string>>(void)> update_callback;
		std::function<Action(void)> action_callback;
		SDL_Scancode kb_shortcut;
	};

	struct TextState : public AbstractButtonState {
        SDL_Texture* font_texture;

        explicit TextState(const ElementBlueprint* blueprint);
		~TextState() override;
        Action update_content() override;
        void draw() override;
	};
}

#endif //M2_TEXT_H
