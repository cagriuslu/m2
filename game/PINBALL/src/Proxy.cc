#include <m2g/Proxy.h>

namespace {
	const m2::UiPanelBlueprint mainMenuBlueprint = {};
	const m2::UiPanelBlueprint pauseMenuBlueprint = {};
	const m2::UiPanelBlueprint leftHudBlueprint = {};
	const m2::UiPanelBlueprint rightHudBlueprint = {};
}

const m2::UiPanelBlueprint* m2g::Proxy::MainMenuBlueprint() {
	return &mainMenuBlueprint;
}
const m2::UiPanelBlueprint* m2g::Proxy::PauseMenuBlueprint() {
	return &pauseMenuBlueprint;
}
const m2::UiPanelBlueprint* m2g::Proxy::LeftHudBlueprint() {
	return &leftHudBlueprint;
}
const m2::UiPanelBlueprint* m2g::Proxy::RightHudBlueprint() {
	return &rightHudBlueprint;
}
