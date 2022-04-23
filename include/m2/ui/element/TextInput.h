#ifndef M2_TEXTINPUT_H
#define M2_TEXTINPUT_H

#include "../ElementState.h"
#include <functional>
#include <string>
#include <sstream>

namespace m2::ui::element {
    struct TextInputBlueprint {
        std::string_view initial_text;
        std::function<Action(std::stringstream&)> action_callback;
    };

    struct TextInputState : public ElementState {
        std::stringstream text_input;
        SDL_Texture* font_texture;
        std::string font_texture_str;

        explicit TextInputState(const ElementBlueprint* blueprint);
        ~TextInputState() override;
        Action handle_events(Events& events) override;
        Action update_content() override;
        void draw() override;
    };
}

#endif //M2_TEXTINPUT_H
