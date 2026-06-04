#pragma once
#include <m2/Proxy.h>

namespace m2g {
	class Proxy final : public m2::Proxy {
	public:
		const std::string gameIdentifier = "LockstepSkeleton";
		const std::string gameFriendlyName = "LockstepSkeleton";

		const std::optional<m2::pb::Cards> cards = m2::Build<m2::pb::Cards>([](auto& cards) { cards.mutable_cards()->Add()->set_type(pb::NO_CARD); });

		// UI

		const m2::UiPanelBlueprint* MainMenuBlueprint();
		const m2::UiPanelBlueprint* LeftHudBlueprint();
		const m2::UiPanelBlueprint* RightHudBlueprint();

		void PreLockstepLevelInit(const std::string& name, const m2::pb::Level& level, const m2g::pb::LockstepGameInitParams& gameInitParams);
		void PostLockstepLevelInit(const std::string& name, const m2::pb::Level& level, const m2g::pb::LockstepGameInitParams& gameInitParams);
		void HandleLockstepPlayerInputs(const std::vector<std::deque<pb::LockstepPlayerInput>>&);

		class LevelState final : public m2::Proxy::LevelState {
		public:
			void PostLevelInit();
		};
	};
}
