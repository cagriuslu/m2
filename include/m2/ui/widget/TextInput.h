#ifndef M2_TEXTINPUT_H
#define M2_TEXTINPUT_H

#include "../WidgetState.h"
#include <functional>
#include <string>
#include <sstream>

namespace m2::ui::wdg {
    struct TextInputBlueprint {
        std::string_view initial_text;
        std::function<Action(std::stringstream&)> action_callback;
    };

    struct TextInputState : public WidgetState {
        std::stringstream text_input;
        SDL_Texture* font_texture;
        std::string font_texture_str;

        explicit TextInputState(const WidgetBlueprint* blueprint);
        ~TextInputState() override;
        Action handle_events(Events& events) override;
        Action update_content() override;
        void draw() override;
    };
}

#endif //M2_TEXTINPUT_H
