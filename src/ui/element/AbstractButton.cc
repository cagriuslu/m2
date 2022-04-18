#include <m2/ui/element/AbstractButton.h>
#include <m2/ui/ElementBlueprint.h>
#include <m2/Def.h>

m2::ui::element::AbstractButtonState::AbstractButtonState(const ElementBlueprint *blueprint) : ElementState(blueprint), depressed(false) {}

SDL_Scancode m2::ui::element::AbstractButtonState::get_keyboard_shortcut() const {
	return std::visit(overloaded {
		[](const TextBlueprint& v) -> SDL_Scancode { return v.kb_shortcut; },
		[](const ImageBlueprint& v) -> SDL_Scancode { return v.kb_shortcut; },
		[]([[maybe_unused]] const auto& v) -> SDL_Scancode { return SDL_SCANCODE_UNKNOWN; }
	}, blueprint->variant);
}

void m2::ui::element::AbstractButtonState::set_depressed(bool state) {
	depressed = state;
}

m2::ui::Action m2::ui::element::AbstractButtonState::action() {
	return std::visit(overloaded {
		[](const TextBlueprint& v) { return v.action_callback(); },
		[](const ImageBlueprint& v) { return v.action_callback(); },
		[]([[maybe_unused]] const auto& v) { return Action::CONTINUE; }
	}, blueprint->variant);
}
