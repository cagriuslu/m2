#pragma once
#include <m2/Proxy.h>

namespace m2g {
	class Proxy : public m2::Proxy {
	   public:
		const std::string gameIdentifier = "MINE";
		const m2::VecF gravity{0.0f, 10.0f};
		static constexpr bool worldIsStatic = false;
		const std::vector<m2::ObjectBlueprint> objectBlueprints = {
			{.objectType = pb::DWARF, .defaultSpriteType = pb::DWARF_FULL},
			{.objectType = pb::BLACKSMITH, .defaultSpriteType = pb::BLACKSMITH_FULL},
		};

		const m2::UiPanelBlueprint* MainMenuBlueprint();
		const m2::UiPanelBlueprint* PauseMenuBlueprint();
		const m2::UiPanelBlueprint* LeftHudBlueprint();
		const m2::UiPanelBlueprint* RightHudBlueprint();

		m2::third_party::physics::FixtureDefinition TileFixtureDefinition(pb::SpriteType spriteType);
		void post_tile_create(m2::Object& obj, m2g::pb::SpriteType sprite_type);
		m2::void_expected LoadForegroundObjectFromLevelBlueprint(m2::Object& obj, const m2::VecF& position, float orientation);
	};
}
