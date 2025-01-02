#include <m2/ui/widget/AbstractButton.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/Image.h>
#include <m2/M2.h>

using namespace m2::ui;
using namespace m2::ui::widget;

AbstractButton::AbstractButton(Panel* parent, const WidgetBlueprint *blueprint) :
		Widget(parent, blueprint),
		kb_shortcut(
				std::visit(overloaded {
						[](const TextBlueprint& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const ImageBlueprint& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const CheckboxWithTextBlueprint& v) -> SDL_Scancode { return v.kb_shortcut; },
						[](const auto& v) -> SDL_Scancode { (void)v; return SDL_SCANCODE_UNKNOWN; }
				}, blueprint->variant)
		),
		depressed(false) {}

Action AbstractButton::on_event(Events &events) {
	// Return early if there is no action callback
	const bool has_action_callback = std::visit(overloaded {
			[](const TextBlueprint& v) -> bool { return (bool) v.on_action; },
			[](const ImageBlueprint& v) -> bool { return (bool) v.on_action; },
			[](const CheckboxWithTextBlueprint& v) -> bool { return (bool) v.on_action; },
			[](MAYBE const auto& v) -> bool { return false; }
	}, blueprint->variant);
	if (!has_action_callback) {
		return MakeContinueAction();
	}

	auto run_action = false;
	if (kb_shortcut != SDL_SCANCODE_UNKNOWN && SDL_IsTextInputActive() == false && events.pop_ui_key_press(kb_shortcut)) {
		run_action = true;
	} else {
		if (not depressed) {
			// Check if mouse pressed inside the rect
			if (events.pop_mouse_button_press(MouseButton::PRIMARY, rect())) {
				depressed = true;
			}
		} else {
			// Check if mouse released inside the rect
			if (events.pop_mouse_button_release(MouseButton::PRIMARY, rect())) {
				depressed = false;
				run_action = true;
			} else if (events.pop_mouse_button_release(MouseButton::PRIMARY)) {
				// Check if mouse released outside the rect
				depressed = false;
			}
		}
	}

	return run_action ? trigger_action() : MakeContinueAction();
}

Action AbstractButton::trigger_action() {
	return std::visit(overloaded {
			[&](const TextBlueprint& v) { return v.on_action ? v.on_action(dynamic_cast<const Text&>(*this)) : MakeContinueAction(); },
			[&](const ImageBlueprint& v) { return v.on_action ? v.on_action(dynamic_cast<const Image&>(*this)) : MakeContinueAction(); },
			[&](const CheckboxWithTextBlueprint& v) {
				// Overloading handle_events for CheckboxWithText is too much work, do it here
				auto& checkbox_with_text_state = dynamic_cast<CheckboxWithText&>(*this);
				checkbox_with_text_state._state = !checkbox_with_text_state._state;
				return v.on_action ? v.on_action(checkbox_with_text_state) : MakeContinueAction();
			},
			[](MAYBE const auto& v) { return MakeContinueAction(); }
	}, blueprint->variant);
}
