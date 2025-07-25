#include <cuzn/ui/Server.h>
#include <m2/Game.h>
#include <m2/Log.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/network/Select.h>
#include <m2/network/IpAddressAndPort.h>
#include <numeric>

using namespace m2;
using namespace m2::widget;
using namespace m2g;
using namespace m2g::pb;

static TextBlueprint client_count = {
	.text = "0",
	.wrapped_font_size_in_units = 5.0f,
	.onUpdate = [](MAYBE Text& self) {
		auto ccount = M2_GAME.ServerThread().GetClientCount();
		auto ready_client_count = M2_GAME.ServerThread().GetReadyClientCount();
		auto text = m2::ToString(ready_client_count) + "/" + m2::ToString(ccount);
		// Check if ready to start
		if (ccount != 1 && ccount == ready_client_count) {
			text += " - START!";
		}
		self.set_text(text);

		// Check if the lobby closure is successful
		if (M2_GAME.ServerThread().IsLobbyClosed()) {
			const auto expect_success = M2_GAME.LoadMultiPlayerAsHost(M2_GAME.GetResources().GetLevelsDir() / "Map.json", m2::ToString(ccount));
			m2SucceedOrThrowError(expect_success);
			return MakeClearStackAction();
		}

		return MakeContinueAction();
	},
	.onAction = [](MAYBE const Text& self) -> UiAction {
		if (2 <= M2_GAME.ServerThread().GetClientCount()) {
			LOG_INFO("Enough clients have connected");
			M2_GAME.ServerThread().TryCloseLobby();
		}
		return MakeContinueAction();
	}
};

const UiPanelBlueprint server_lobby = {
	.name = "ServerLobby",
	.w = 160,
	.h = 90,
	.border_width = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		UiWidgetBlueprint{
			.x = 35, .y = 5, .w = 40, .h = 20,
			.border_width = 0,
			.variant = TextBlueprint{
				.text = "LISTENING ON:",
				.horizontal_alignment = m2::TextHorizontalAlignment::RIGHT,
				.vertical_alignment = m2::TextVerticalAlignment::CENTER,
				.wrapped_font_size_in_units = 5.0f
			}
		},
		UiWidgetBlueprint{
			.x = 85, .y = 5, .w = 60, .h = 20,
			.border_width = 0,
			.variant = TextBlueprint{
				.horizontal_alignment = m2::TextHorizontalAlignment::LEFT,
				.vertical_alignment = m2::TextVerticalAlignment::CENTER,
				.wrapped_font_size_in_units = 5.0f,
				.onCreate = [](Text& self) {
					if (auto addresses = m2::network::get_ip_addresses()) {
						auto addresses_str = std::accumulate(addresses->begin(), addresses->end(), std::string{}, [](std::string&& ss, const std::string& s) {
							return ss.empty() ? s : (ss + " " + s);
						});
						self.set_text(addresses_str);
					}
				}
			}
		},
		UiWidgetBlueprint{
			.x = 35, .y = 35, .w = 40, .h = 10,
			.border_width = 0,
			.variant = TextBlueprint{
				.text = "CLIENT COUNT:",
				.horizontal_alignment = m2::TextHorizontalAlignment::RIGHT,
				.wrapped_font_size_in_units = 5.0f,
			}
		},
		UiWidgetBlueprint{
			.x = 85, .y = 35, .w = 40, .h = 10,
			.variant = client_count
		},
		UiWidgetBlueprint{
			.x = 60, .y = 55, .w = 40, .h = 10,
			.variant = TextBlueprint{
				.text = "ADD BOT",
				.wrapped_font_size_in_units = 5.0f,
				.onAction = [](MAYBE const m2::widget::Text& self) -> m2::UiAction {
					M2_GAME.AddBot();
					return MakeContinueAction();
				}
			}
		},
		UiWidgetBlueprint{
			.x = 60, .y = 75, .w = 40, .h = 10,
			.variant = TextBlueprint{
				.text = "CANCEL",
				.wrapped_font_size_in_units = 5.0f,
				.onAction = [](MAYBE const Text& self) -> UiAction {
					M2_GAME.LeaveGame();
					// TODO kill bots if any
					return MakeReturnAction();
				}
			}
		}
	}
};
