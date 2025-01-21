#pragma once

#include <m2/Proxy.h>

namespace m2g {
	class Proxy : public m2::Proxy {
	public:

		const std::string game_identifier = "PINBALL";
		const std::string game_friendly_name = "Pin Ball Evolution";

		const m2::UiPanelBlueprint* MainMenuBlueprint();
		const m2::UiPanelBlueprint* PauseMenuBlueprint();
		const m2::UiPanelBlueprint* LeftHudBlueprint();
		const m2::UiPanelBlueprint* RightHudBlueprint();
	};
}  // namespace m2g
