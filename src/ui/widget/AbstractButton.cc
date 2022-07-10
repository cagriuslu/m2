#include <m2/ui/widget/AbstractButton.h>
#include <m2/ui/WidgetBlueprint.h>
#include <m2/Def.h>

using namespace m2::ui;
using namespace m2::ui::wdg;

AbstractButtonState::AbstractButtonState(const WidgetBlueprint *blueprint) :
	WidgetState(blueprint),
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

    if (kb_shortcut != SDL_SCANCODE_UNKNOWN && SDL_IsTextInputActive() == false && events.pop_ui_key_press(kb_shortcut)) {
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
