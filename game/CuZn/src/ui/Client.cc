#include <cuzn/ui/Client.h>
#include <m2/ui/widget/Text.h>
#include <m2/ui/widget/TextInput.h>
#include <m2/Game.h>
#include <m2/Log.h>

using namespace m2;
using namespace m2::widget;
using namespace m2g;
using namespace m2g::pb;

static TextBlueprint client_status = {
	.text = "CONNECTING",
	.horizontal_alignment = m2::TextHorizontalAlignment::LEFT,
	.wrapped_font_size_in_units = 5.0f,
	.onUpdate = [](MAYBE Text& self) {
		if (M2_GAME.TurnBasedRealClientThread().is_connected()) {
			self.set_text("CONNECTED");
		} else if (M2_GAME.TurnBasedRealClientThread().is_ready()) {
			self.set_text("READY");
		} else if (M2_GAME.TurnBasedRealClientThread().is_started()) {
			// Start the game
			auto player_count = M2_GAME.TurnBasedRealClientThread().total_player_count();
			const auto expect_success =
				M2_GAME.LoadMultiPlayerAsGuest(M2_GAME.GetResources().GetLevelsDir() / "Map.json", m2::ToString(player_count));
			m2SucceedOrThrowError(expect_success);
			return MakeClearStackAction();
		} else if (M2_GAME.TurnBasedRealClientThread().IsQuit()) {
			self.set_text("SERVER FULL");
		} else {
			self.set_text("CONNECTING...");
		}
		return MakeContinueAction();
	}
};

static TextBlueprint ready_button = {
	.text = "...",
	.wrapped_font_size_in_units = 5.0f,
	.onUpdate = [](MAYBE Text& self) {
		if (M2_GAME.TurnBasedRealClientThread().is_connected()) {
			self.set_text("SET READY");
		} else if (M2_GAME.TurnBasedRealClientThread().is_ready()) {
			self.set_text("CLEAR READY");
		} else {
			self.set_text("...");
		}
		return MakeContinueAction();
	},
	.onAction = [](MAYBE const Text& self) -> UiAction {
		if (M2_GAME.TurnBasedRealClientThread().is_connected()) {
			M2_GAME.TurnBasedRealClientThread().set_ready(true);
		} else if (M2_GAME.TurnBasedRealClientThread().is_ready()) {
			M2_GAME.TurnBasedRealClientThread().set_ready(false);
		}
		return MakeContinueAction();
	}
};

static const UiPanelBlueprint client_lobby = {
	.name = "ClientLobby",
	.w = 160,
	.h = 90,
	.border_width = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		UiWidgetBlueprint{
			.x = 40, .y = 20, .w = 40, .h = 10,
			.border_width = 0,
			.variant = TextBlueprint{
				.text = "STATUS:",
				.wrapped_font_size_in_units = 5.0f
			}
		},
		UiWidgetBlueprint{
			.x = 80, .y = 20, .w = 40, .h = 10,
			.border_width = 0,
			.variant = client_status
		},
		UiWidgetBlueprint{
			.x = 60, .y = 40, .w = 40, .h = 10,
			.variant = ready_button
		},
		UiWidgetBlueprint{
			.x = 60, .y = 60, .w = 40, .h = 10,
			.variant = TextBlueprint{
				.text = "CANCEL",
				.wrapped_font_size_in_units = 5.0f,
				.onAction = [](MAYBE const Text& self) -> UiAction {
					M2_GAME.LeaveGame();
					return MakeReturnAction();
				}
			}
		},
	}
};

const UiPanelBlueprint ip_port_form = {
	.name = "IpPortForm",
	.w = 160,
	.h = 90,
	.border_width = 0,
	.background_color = {20, 20, 20, 255},
	.widgets = {
		UiWidgetBlueprint{
			.x = 40, .y = 20, .w = 40, .h = 10,
			.border_width = 0,
			.variant = TextBlueprint{
				.text = "ENTER IP:",
				.horizontal_alignment = m2::TextHorizontalAlignment::RIGHT,
				.wrapped_font_size_in_units = 5.0f,
			}
		},
		UiWidgetBlueprint{
			.initially_focused = true,
			.x = 80, .y = 20, .w = 40, .h = 10,
			.variant = TextInputBlueprint{
				.initial_text = "127.0.0.1"
			}
		},
		UiWidgetBlueprint{
			.x = 60, .y = 40, .w = 40, .h = 10,
			.variant = TextBlueprint{
				.text = "CONNECT",
				.wrapped_font_size_in_units = 5.0f,
				.onAction = [](MAYBE const widget::Text& self) {
					auto* ip_input_widget = self.Parent().FindWidget<TextInput>();
					M2_GAME.JoinGame(m2::mplayer::Type::TurnBased, ip_input_widget->text_input());
					return m2::UiPanel::create_and_run_blocking(&client_lobby);
				}
			}
		},
		UiWidgetBlueprint{
			.x = 60, .y = 60, .w = 40, .h = 10,
			.variant = TextBlueprint{
				.text = "CANCEL",
				.wrapped_font_size_in_units = 5.0f,
				.onAction = [](MAYBE const Text& self) -> UiAction { return MakeReturnAction(); }
			}
		}
	}
};
