#include <m2/ui/widget/AbstractButton.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/M2.h>

using namespace m2::ui;
using namespace m2::ui::widget;

AbstractButton::AbstractButton(const WidgetBlueprint *blueprint) :
		Widget(blueprint),
		kb_shortcut(
				std::visit(overloaded {
						[](const TextBlueprint& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const ImageBlueprint& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const CheckboxWithTextBlueprint& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const auto& v) -> SDL_Scancode { (void)v; return SDL_SCANCODE_UNKNOWN; }
				}, blueprint->variant)
		),
		depressed(false) {}

Action AbstractButton::handle_events(Events &events) {
	// Return early if there is no action callback
	bool has_action_callback = std::visit(overloaded {
			[](const TextBlueprint& v) -> bool { return (bool) v.action_callback; },
			[](const ImageBlueprint& v) -> bool { return (bool) v.action_callback; },
			[](const CheckboxWithTextBlueprint& v) -> bool { return (bool) v.action_callback; },
			[](MAYBE const auto& v) -> bool { return false; }
	}, blueprint->variant);
	if (!has_action_callback) {
		return Action::CONTINUE;
	}

	bool run_action{};
	if (kb_shortcut != SDL_SCANCODE_UNKNOWN && SDL_IsTextInputActive() == false && events.pop_ui_key_press(kb_shortcut)) {
		run_action = true;
	} else {
		if (not depressed) {
			// Check if mouse pressed inside the rect
			if (events.pop_mouse_button_press(MouseButton::PRIMARY, RectI{rect_px})) {
				depressed = true;
			}
		} else {
			// Check if mouse released inside the rect
			if (events.pop_mouse_button_release(MouseButton::PRIMARY, RectI{rect_px})) {
				depressed = false;
				run_action = true;
			} else if (events.pop_mouse_button_release(MouseButton::PRIMARY)) {
				// Check if mouse released outside the rect
				depressed = false;
			}
		}
	}
	if (run_action) {
		return std::visit(overloaded {
				[](const TextBlueprint& v) { return v.action_callback ? v.action_callback() : Action::CONTINUE; },
				[](const ImageBlueprint& v) { return v.action_callback ? v.action_callback() : Action::CONTINUE; },
				[&](const CheckboxWithTextBlueprint& v) {
					// Overloading handle_events for CheckboxWithText is too much work, do it here
					auto checkbox_with_text_state = dynamic_cast<CheckboxWithText*>(this);
					checkbox_with_text_state->state = !checkbox_with_text_state->state;
					return v.action_callback ? v.action_callback(checkbox_with_text_state->state) : Action::CONTINUE;
				},
				[](MAYBE const auto& v) { return Action::CONTINUE; }
		}, blueprint->variant);
	} else {
		return Action::CONTINUE;
	}
}
