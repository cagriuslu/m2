#include <m2g/Proxy.h>
#include <m2/Game.h>
#include <m2/ui/widget/TextSelection.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/Layout.h>
#include <m2/ui/UiAction.h>
#include <m2/Log.h>

namespace {
	m2::UiPanelBlueprint CreateServerLobbyBlueprint(const m2::network::IpAddress multicastInterface) {
		return m2::UiPanelBlueprint{
			.name = "ServerLobby",
			.w = 160, .h = 90,
			.border_width = 0.0f,
			.background_color = {0, 0, 0, 255},
			.widgets = *m2::MakeVerticalLayout(160, 90, 1, {
				m2::DynamicSpacer{},
				m2::UiWidgetBlueprint{
					.name = "AddBotButton",
					.x = 1, .h = 10,
					.variant = m2::widget::TextBlueprint{
						.text = "ADD BOT",
						.onAction = [](const m2::widget::Text&) -> m2::UiAction {
							M2_GAME.AddLockstepBot();
							return m2::MakeContinueAction();
						}
					}
				},
				m2::UiWidgetBlueprint{
					.name = "StartButton",
					.x = 1, .h = 10,
					.variant = m2::widget::TextBlueprint{
						.text = "START",
						.onCreate = [=](const m2::widget::Text&) {
							M2_GAME.HostLockstepGame(4, multicastInterface);
							std::this_thread::sleep_for(std::chrono::milliseconds{100});
						},
						.onAction = [](const m2::widget::Text&) -> m2::UiAction {
							LOG_INFO("Will freeze lobby");
							M2_GAME.GetLockstepServerActor().TryFreezeLobby({});

							do {
								std::this_thread::sleep_for(std::chrono::milliseconds{50});
								// This check is expensive, avoid doing it for every frame.
							} while (M2_GAME.GetLockstepHostClientActor().GetGameInitParams() == nullptr);
							LOG_INFO("Lobby is frozen for the client");

							const auto initResult = M2_GAME.LoadLockstep(m2::pb::Level{}, "NONAME", *M2_GAME.GetLockstepHostClientActor().GetGameInitParams());
							m2SucceedOrThrowError(initResult);

							return m2::MakeClearStackAction();
						}
					}
				},
				m2::DynamicSpacer{},
			})
		};
	}

	const m2::UiPanelBlueprint gClientLobby = {
		.name = "ClientLobby",
		.w = 160, .h = 90,
		.border_width = 0.0f,
		.background_color = {0, 0, 0, 255},
		.widgets = *m2::MakeVerticalLayout(160, 90, 1, {
			m2::DynamicSpacer{},
			m2::UiWidgetBlueprint{
				.x = 80, .y = 20, .w = 40, .h = 10,
				.border_width = 0,
				.variant = m2::widget::TextBlueprint{
					.text = "CONNECTING",
					.horizontal_alignment = m2::TextHorizontalAlignment::CENTER,
					.onUpdate = [](MAYBE m2::widget::Text& self) -> m2::UiAction {
						if (M2_GAME.GetLockstepGuestClientActor().IsWaitingInLobby()) {
							self.set_text("WAITING IN LOBBY");
						} else if (const auto* gameInitParams = M2_GAME.GetLockstepGuestClientActor().GetGameInitParams()) {
							LOG_INFO("Lobby is frozen");
							const auto initResult = M2_GAME.LoadLockstep(m2::pb::Level{}, "NONAME", *gameInitParams);
							m2SucceedOrThrowError(initResult);
							return m2::MakeClearStackAction();
						} else {
							self.set_text("CONNECTING...");
						}
						// GetGameInitParams is expensive, avoid doing it for every frame.
						std::this_thread::sleep_for(std::chrono::milliseconds{10});
						return m2::MakeContinueAction();
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 60, .y = 40, .w = 40, .h = 10,
				.variant = m2::widget::TextBlueprint{
					.text = "...",
					.onUpdate = [](MAYBE m2::widget::Text& self) {
						if (M2_GAME.GetLockstepGuestClientActor().IsWaitingInLobby() && M2_GAME.GetLockstepGuestClientActor().GetLastSetReadyState() == false) {
							self.set_text("SET READY");
						} else if (M2_GAME.GetLockstepGuestClientActor().GetLastSetReadyState()) {
							self.set_text("CLEAR READY");
						} else {
							self.set_text("...");
						}
						return m2::MakeContinueAction();
					},
					.onAction = [](MAYBE const m2::widget::Text& self) -> m2::UiAction {
						if (M2_GAME.GetLockstepGuestClientActor().IsWaitingInLobby() && M2_GAME.GetLockstepGuestClientActor().GetLastSetReadyState() == false) {
							M2_GAME.GetLockstepGuestClientActor().SetReadyState(true);
						} else if (M2_GAME.GetLockstepGuestClientActor().GetLastSetReadyState()) {
							M2_GAME.GetLockstepGuestClientActor().SetReadyState(false);
						}
						return m2::MakeContinueAction();
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 60, .y = 60, .w = 40, .h = 10,
				.variant = m2::widget::TextBlueprint{
					.text = "CANCEL",
					.onAction = [](MAYBE const m2::widget::Text& self) -> m2::UiAction {
						M2_GAME.LeaveGame();
						return m2::MakeReturnAction();
					}
				}
			},
			m2::DynamicSpacer{},
		})
	};

	const m2::UiPanelBlueprint gSelectGameParams{
		.name = "GameParams",
		.w = 160, .h = 90,
		.border_width = 0.0f,
		.background_color = {0, 0, 0, 255},
		.widgets = *m2::MakeVerticalLayout(160, 90, 1, {
			m2::DynamicSpacer{},
			m2::UiWidgetBlueprint{
				.name = "LocalInterfaceSelection",
				.x = 1, .h = 40,
				.variant = m2::widget::TextSelectionBlueprint{
					.line_count = 4,
					.onCreate = [](m2::widget::TextSelection& self) {
						m2::widget::TextSelectionBlueprint::Options options;
						for (const auto& interface : m2::network::InferLanAddresses(*m2::network::GetInterfaces())) {
							options.emplace_back(m2::widget::TextSelectionBlueprint::Option{.text = std::format("{}", interface), .return_value = interface});
						}
						self.SetOptions(std::move(options));
					}
				}
			},
			m2::UiWidgetBlueprint{
				.name = "OpenLobbyButton",
				.x = 1, .h = 10,
				.variant = m2::widget::TextBlueprint{
					.text = "OPEN LOBBY",
					.onAction = [](const m2::widget::Text& self) -> m2::UiAction {
						// TODO gather game parameters
						const auto* localInterfaceSelection = self.Parent().FindWidget<m2::widget::TextSelection>("LocalInterfaceSelection");
						const auto selectedOptions = localInterfaceSelection->GetSelectedOptions();
						if (selectedOptions.empty()) {
							return m2::MakeContinueAction();
						}
						const auto serverLobbyBlueprint = CreateServerLobbyBlueprint(std::get<m2::network::IpAddress>(selectedOptions[0]));
						return ConvertReturnActionToContinue(m2::UiPanel::create_and_run_blocking(&serverLobbyBlueprint));
					}
				}
			},
			m2::DynamicSpacer{}
		})
	};

	const m2::UiPanelBlueprint DiscoverGameServer{
		.name = "DiscoverGameServer",
		.w = 160, .h = 90,
		.border_width = 0.0f,
		.background_color = {0, 0, 0, 255},
		.widgets = *m2::MakeVerticalLayout(160, 90, 1, {
			m2::DynamicSpacer{},
			m2::UiWidgetBlueprint{
				.name = "GameServerSelection",
				.x = 1, .h = 40,
				.variant = m2::widget::TextSelectionBlueprint{
					.line_count = 4,
					.onCreate = [](m2::widget::TextSelection&) {
						M2_GAME.EnableServerDiscovery();
					},
					.onUpdate = [](m2::widget::TextSelection& self) {
						auto& networkDiscovery = M2_GAME.GetNetworkDiscoveryActorInterface();
						if (const auto peers = networkDiscovery.GetDiscoveredPeers(); m2::I(peers.size()) != self.GetOptionCount()) {
							m2::widget::TextSelectionBlueprint::Options options;
							for (const auto& peer : peers) {
								options.emplace_back(m2::widget::TextSelectionBlueprint::Option{.text = std::format("{}", peer), .return_value = peer});
							}
							self.SetOptions(std::move(options));
						}
						return m2::MakeContinueAction();
					},
					.onDestroy = [] {
						M2_GAME.DisableServerDiscovery();
					}
				}
			},
			m2::UiWidgetBlueprint{
				.x = 1, .h = 10,
				.variant = m2::widget::TextBlueprint{
					.text = "CONNECT",
					.onAction = [](const m2::widget::Text& self) -> m2::UiAction {
						const auto* gameServerSelection = self.Parent().FindWidget<m2::widget::TextSelection>("GameServerSelection");
						const auto selectedOptions = gameServerSelection->GetSelectedOptions();
						if (selectedOptions.empty()) {
							return m2::MakeContinueAction();
						}
						const auto selectedOption = std::get<m2::network::IpAddressAndPort>(selectedOptions[0]);
						M2_GAME.JoinLockstepGame(selectedOption);
						return ConvertReturnActionToContinue(m2::UiPanel::create_and_run_blocking(&gClientLobby));
					}
				}
			},
			m2::DynamicSpacer{}
		})
	};

	const m2::UiPanelBlueprint gMainMenuBlueprint{
		.name = "MainMenu",
		.w = 160, .h = 90,
		.border_width = 0.0f,
		.background_color = {0, 0, 0, 255},
		.widgets = *m2::MakeVerticalLayout(160, 90, 1, {
			m2::DynamicSpacer{},
			m2::UiWidgetBlueprint{
				.name = "JoinButton",
				.x = 1, .h = 10,
				.variant = m2::widget::TextBlueprint{
					.text = "JOIN",
					.onAction = [](const m2::widget::Text&) -> m2::UiAction {
						return ConvertReturnActionToContinue(m2::UiPanel::create_and_run_blocking(&DiscoverGameServer));
					}
				}
			},
			m2::UiWidgetBlueprint{
				.name = "HostButton",
				.x = 1, .h = 10,
				.variant = m2::widget::TextBlueprint{
					.text = "HOST",
					.onAction = [](const m2::widget::Text&) {
						return m2::ConvertReturnActionToContinue(m2::UiPanel::create_and_run_blocking(&gSelectGameParams));
					}
				}
			},
			m2::UiWidgetBlueprint{
				.name = "QuitButton",
				.x = 1, .h = 10,
				.variant = m2::widget::TextBlueprint{
					.text = "QUIT",
					.onAction = [](const m2::widget::Text&) {
						return m2::MakeQuitAction();
					}
				}
			},
			m2::DynamicSpacer{},
		})
	};
}

const m2::UiPanelBlueprint* m2g::Proxy::MainMenuBlueprint() { return &gMainMenuBlueprint; }
const m2::UiPanelBlueprint* m2g::Proxy::LeftHudBlueprint() { return nullptr; }
const m2::UiPanelBlueprint* m2g::Proxy::RightHudBlueprint() { return nullptr; }

void m2g::Proxy::PreLockstepLevelInit(const std::string&, const m2::pb::Level&, const m2g::pb::LockstepGameInitParams&) {}
void m2g::Proxy::PostLockstepLevelInit(const std::string&, const m2::pb::Level&, const pb::LockstepGameInitParams&) {
	M2_LEVEL.GetProxyLevelState().PostLevelInit();
	M2_GAME.EnableLevelSaver("LockstepSkeleton.db");
}
void m2g::Proxy::HandleLockstepPlayerInputs(const std::vector<std::deque<pb::LockstepPlayerInput>>&) {}
std::deque<m2g::pb::LockstepPlayerInput> m2g::Proxy::GenerateLockstepBotInput(int) {
	// No-op bot: stays idle. Meaningful AI can read M2_LEVEL here and emit inputs for the bot's player index.
	return {};
}

void m2g::Proxy::LevelState::PostLevelInit() {
	const auto playerCount = M2_GAME.GetTotalPlayerCount();
	if (playerCount == 1) {
		object::Player::Create(M2_GAME.GetSelfIndex() == 0, M2_LEVEL.multiPlayerObjectIds);
	} else if (playerCount == 2) {
		object::Player::Create(M2_GAME.GetSelfIndex() == 0, M2_LEVEL.multiPlayerObjectIds);
		object::Player::Create(M2_GAME.GetSelfIndex() == 1, M2_LEVEL.multiPlayerObjectIds);
	}
	M2_LEVEL.playerId = M2_LEVEL.multiPlayerObjectIds[M2_GAME.GetSelfIndex()];
}
