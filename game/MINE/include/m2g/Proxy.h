#pragma once
#include <m2/Proxy.h>

namespace m2g {
	class Proxy : public m2::Proxy {
	   public:
		const std::string game_identifier = "MINE";
		const bool gravity = true;
		const bool world_is_static = false;

		const m2::UiPanelBlueprint* MainMenuBlueprint();
		const m2::UiPanelBlueprint* PauseMenuBlueprint();
		const m2::UiPanelBlueprint* LeftHudBlueprint();
		const m2::UiPanelBlueprint* RightHudBlueprint();

		void post_tile_create(m2::Object& obj, m2g::pb::SpriteType sprite_type);
		m2::void_expected init_level_blueprint_fg_object(m2::Object& obj);
	};
}
