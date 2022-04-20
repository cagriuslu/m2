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

Action AbstractButtonState::handle_events(const Events &events) {
    bool run_action = false;

    if (kb_shortcut != SDL_SCANCODE_UNKNOWN && SDL_IsTextInputActive() == false && events.raw_key_down[kb_shortcut]) {
        run_action = true;
    } else {
        SDL_Point mouse_position = {events.mouse_position.x, events.mouse_position.y};
        if (SDL_PointInRect(&mouse_position, &rect_px)) {
            if (not depressed && events.mouse_buttons_pressed[u(MouseButton::PRIMARY)]) {
                depressed = true;
            } else if (depressed && events.mouse_buttons_released[u(MouseButton::PRIMARY)]) {
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
