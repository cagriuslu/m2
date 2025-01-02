#include <cuzn/ui/ActionNotification.h>
#include <m2/ui/Panel.h>
#include <m2/ui/widget/TextBlueprint.h>
#include <m2/ui/widget/Text.h>
#include <cuzn/ui/Detail.h>
#include <m2/Log.h>
#include <m2/Game.h>

using namespace m2;
using namespace m2::ui;

void display_action_notification(const m2g::pb::ServerCommand::ActionNotification& action_notification) {
	LOG_INFO("Displaying action notification", action_notification.player_index(), action_notification.notification());

	auto blueprint = PanelBlueprint{
		.w = 60, .h = 60,
		.background_color = {0, 0, 0, 255},
		.widgets = {
			WidgetBlueprint{
				.x = 5, .y = 5, .w = 50, .h = 5,
				.border_width = 0,
				.variant = widget::TextBlueprint{
					.text = generate_player_name(action_notification.player_index()),
					.wrapped_font_size_in_units = 3.0f,
					.color = generate_player_color(action_notification.player_index())
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 15, .w = 50, .h = 30,
				.border_width = 0,
				.variant = widget::TextBlueprint{
					.text = action_notification.notification(),
					.horizontal_alignment = TextHorizontalAlignment::LEFT,
					.vertical_alignment = TextVerticalAlignment::TOP,
					.wrapped_font_size_in_units = 3.0f
				}
			},
			WidgetBlueprint{
				.x = 5, .y = 50, .w = 50, .h = 5,
				.variant = widget::TextBlueprint{
					.text = "OK",
					.wrapped_font_size_in_units = 3.0f,
					.kb_shortcut = SDL_SCANCODE_RETURN,
					.on_action = [](MAYBE const widget::Text& self) -> Action {
						return MakeReturnAction();
					}
				}
			}
		}
	};

	// Play sound
	M2_GAME.audio_manager->play(&M2_GAME.songs[m2g::pb::SONG_NOTIFICATION_SOUND], m2::AudioManager::ONCE);

	Panel::create_and_run_blocking(&blueprint, RectF{0.15f, 0.15f, 0.7f, 0.7f});
}
