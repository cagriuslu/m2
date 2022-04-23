#include <m2/ui/element/AbstractButton.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/Def.h>

using namespace m2::ui;
using namespace m2::ui::element;

AbstractButtonState::AbstractButtonState(const ElementBlueprint *blueprint) :
        ElementState(blueprint),
        kb_shortcut(
            std::visit(overloaded {
                [](const TextBlueprint& v) -> SDL_Scancode { return v.kb_shortcut; },
                [](const ImageBlueprint& v) -> SDL_Scancode { return v.kb_shortcut; },
                [](const auto& v) -> SDL_Scancode { (void)v; return SDL_SCANCODE_UNKNOWN; }
            }, blueprint->variant)
        ),
        depressed(false) {}

Action AbstractButtonState::handle_events(Events &events) {
    bool run_action = false;

    if (kb_shortcut != SDL_SCANCODE_UNKNOWN && SDL_IsTextInputActive() == false && events.is_sdl_key_down(kb_shortcut)) {
        run_action = true;
    } else {
		auto mouse_position = events.mouse_position();
        SDL_Point sdl_mouse_position = {mouse_position.x, mouse_position.y};
        if (SDL_PointInRect(&sdl_mouse_position, &rect_px)) {
            if (not depressed && events.pop_mouse_button_press(MouseButton::PRIMARY)) {
                depressed = true;
            } else if (depressed && events.pop_mouse_button_release(MouseButton::PRIMARY)) {
                run_action = true;
            }
        }
    }

    if (run_action) {
        depressed = false;
        return std::visit(overloaded {
                [](const TextBlueprint& v) { return v.action_callback ? v.action_callback() : Action::CONTINUE; },
                [](const ImageBlueprint& v) { return v.action_callback ? v.action_callback() : Action::CONTINUE; },
                [](const auto& v) { (void)v; return Action::CONTINUE; }
        }, blueprint->variant);
    } else {
        return Action::CONTINUE;
    }
}
