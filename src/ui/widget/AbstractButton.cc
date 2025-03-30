#include <m2/ui/widget/AbstractButton.h>
#include <m2/ui/widget/CheckboxWithText.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/Image.h>
#include <m2/M2.h>

using namespace m2;
using namespace m2::widget;

AbstractButton::AbstractButton(UiPanel* parent, const UiWidgetBlueprint *blueprint) :
		UiWidget(parent, blueprint),
		keyboardShortcut(
				std::visit(overloaded {
						[](const TextBlueprint& v) -> m2g::pb::KeyType { return v.keyboardShortcut; },
						[](const ImageBlueprint& v) -> m2g::pb::KeyType { return v.keyboardShortcut; },
						[](const CheckboxWithTextBlueprint& v) -> m2g::pb::KeyType { return v.keyboardShortcut; },
						[](MAYBE const auto& v) -> m2g::pb::KeyType { return {}; }
				}, blueprint->variant)
		),
		depressed(false) {}

UiAction AbstractButton::HandleEvents(Events &events) {
	// Return early if there is no action callback
	const bool has_action_callback = std::visit(overloaded {
			[](const TextBlueprint& v) -> bool { return (bool) v.onAction; },
			[](const ImageBlueprint& v) -> bool { return (bool) v.onAction; },
			[](MAYBE const CheckboxWithTextBlueprint& v) -> bool { return true; }, // Checkbox might change state even if there isn't an onAction callback.
			[](MAYBE const auto& v) -> bool { return false; }
	}, blueprint->variant);
	if (!has_action_callback) {
		return MakeContinueAction();
	}

	auto run_action = false;
	if (keyboardShortcut && not SDL_IsTextInputActive() && events.PopKeyPress(keyboardShortcut)) {
		run_action = true;
	} else {
		if (not depressed) {
			// Check if mouse pressed inside the rect
			if (events.PopMouseButtonPress(MouseButton::PRIMARY, Rect())) {
				depressed = true;
			}
		} else {
			// Check if mouse released inside the rect
			if (events.PopMouseButtonRelease(MouseButton::PRIMARY, Rect())) {
				depressed = false;
				run_action = true;
			} else if (events.PopMouseButtonRelease(MouseButton::PRIMARY)) {
				// Check if mouse released outside the rect
				depressed = false;
			}
		}
	}

	return run_action ? trigger_action() : MakeContinueAction();
}

UiAction AbstractButton::trigger_action() {
	return std::visit(overloaded {
			[&](const TextBlueprint& v) { return v.onAction ? v.onAction(dynamic_cast<const Text&>(*this)) : MakeContinueAction(); },
			[&](const ImageBlueprint& v) { return v.onAction ? v.onAction(dynamic_cast<const Image&>(*this)) : MakeContinueAction(); },
			[&](const CheckboxWithTextBlueprint& v) {
				// Overloading HandleEvents for CheckboxWithText is too much work, do it here
				auto& checkbox_with_text_state = dynamic_cast<CheckboxWithText&>(*this);
				checkbox_with_text_state._state = !checkbox_with_text_state._state;
				return v.onAction ? v.onAction(checkbox_with_text_state) : MakeContinueAction();
			},
			[](MAYBE const auto& v) { return MakeContinueAction(); }
	}, blueprint->variant);
}
