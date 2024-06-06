#include <m2/m3/VecF.h>
#include <m2/Proxy.h>
#include "m2/Game.h"
#include "m2/sdl/Detail.h"
#include "m2/sdl/Stopwatch.h"
#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <m2/Log.h>
#include <m2/command_line/SpriteSheets.h>
#include <m2/detail/ArgumentParser.h>

#define BREAK_IF_QUIT() if (M2_GAME.quit) break

using namespace m2;

int main(const int argc, char **argv) {
	init_thread_logger("MN");
	INFO_FN();

	auto arg_list = to_argument_list(argc, argv);
	if (auto log_level_opt = parse_argument(arg_list, "log-level"); log_level_opt) {
		if (not LogLevel_Parse(*log_level_opt, &current_log_level)) {
			LOG_WARN("Invalid log level", *log_level_opt);
		}
		LOG_INFO("New log level", current_log_level);
	}
	if (auto silent_opt = parse_argument(arg_list, "silent"); silent_opt) {
		LOG_INFO("Silent");
		silent = true;
	}
	if (auto slowdown_opt = parse_argument(arg_list, "slowdown"); slowdown_opt) {
		if (auto const slowdown_factor = strtol(slowdown_opt->c_str(), nullptr, 0); 1 <= slowdown_factor) {
			time_slowdown_factor = static_cast<int>(slowdown_factor);
			LOG_INFO("New slowdown factor", time_slowdown_factor);
		} else {
			LOG_WARN("Invalid slowdown factor", *slowdown_opt);
		}
	}
	if (auto sprite_sheets_opt = parse_argument(arg_list, "sprite_sheets"); sprite_sheets_opt) {
		LOG_INFO("Generating sprite sheets");
		printf("%s\n", generate_sprite_sheets_skeleton().c_str());
		return 0;
	}
	if (auto console_opt = parse_argument(arg_list, "console"); console_opt) {
		console_command = *console_opt;
		LOG_INFO("Console command", *console_opt);
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		LOG_FATAL("SDL_Init error", SDL_GetError());
		return -1;
	}
	LOG_DEBUG("SDL initialized");
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		LOG_FATAL("IMG_Init error", IMG_GetError());
		return -1;
	}
	LOG_DEBUG("SDL_image initialized");
	if (TTF_Init() != 0) {
		LOG_FATAL("TTF_Init error", TTF_GetError());
		return -1;
	}
	LOG_DEBUG("SDL_ttf initialized");

	Game::create_instance();
	M2_GAME.add_pause_ticks(sdl::get_ticks()); // Add initialization duration as pause ticks

	LOG_DEBUG("Executing main menu...");
	if (m2::ui::State::create_execute_sync(M2G_PROXY.main_menu()).is_quit()) {
		LOG_INFO("Main menu returned QUIT");
		return 0;
	}
	LOG_DEBUG("Main menu returned");

	sdl::Stopwatch since_last_phy(M2_GAME.pause_ticks);
	sdl::Stopwatch since_last_gfx(M2_GAME.pause_ticks);
	sdl::Stopwatch since_last_fps(M2_GAME.pause_ticks);
	unsigned phy_count{}, gfx_count{}, last_phy_count = UINT_MAX;
	LOG_DEBUG("Initial pause ticks", M2_GAME.pause_ticks);
	while (!M2_GAME.quit) {
		M2_LEVEL.begin_game_loop();

		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (last_phy_count) {
			// Clear the events only if the physics step has executed
			// Otherwise some keys/buttons may not have been handled
			M2_GAME.events.clear();
		}
		if (M2_GAME.events.gather()) {
			M2_GAME.handle_quit_event();
			M2_GAME.handle_window_resize_event();
			M2_GAME.handle_console_event();
			M2_GAME.handle_menu_event();
			M2_GAME.handle_hud_events();
			M2_GAME.execute_deferred_actions();
		}
		BREAK_IF_QUIT();
		M2_GAME.reset_mouse_position();

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// PHYSICS ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		for (last_phy_count = 0;
			// Up to 4 times, if enough time has passed since last phy, if game hasn't quit
			last_phy_count < 4 && M2_GAME.phy_period_ticks <= since_last_phy.measure() && !M2_GAME.quit;
			// Increment phy counters, subtract period from stopwatch
			++last_phy_count, ++phy_count, since_last_phy.subtract_from_lap(M2_GAME.phy_period_ticks)) {
			// Advance time by M2_GAME.phy_period
			M2_GAME._delta_time_s = M2_GAME.phy_period;

			M2_GAME.execute_pre_step();
			M2_GAME.execute_deferred_actions();
			M2_GAME.update_characters();
			M2_GAME.execute_deferred_actions();
			M2_GAME.execute_step();
			M2_GAME.execute_deferred_actions();
			M2_GAME.execute_post_step();
			M2_GAME.execute_deferred_actions();
			M2_GAME.update_sounds();
			M2_GAME.execute_deferred_actions();
			M2_GAME.recalculate_directional_audio();
		}
		if (last_phy_count == 4) {
			since_last_phy.new_lap();
		}
		BREAK_IF_QUIT();

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Measure and advance time
		since_last_gfx.measure();
		M2_GAME._delta_time_s = static_cast<float>(since_last_gfx.last()) / 1000.0f;

		M2_GAME.execute_pre_draw();
		M2_GAME.update_hud_contents();
		M2_GAME.clear_back_buffer();
		M2_GAME.draw_background();
		M2_GAME.draw_foreground();
		M2_GAME.draw_lights();
		M2_GAME.execute_post_draw();
		M2_GAME.debug_draw();
		M2_GAME.draw_hud();
		M2_GAME.draw_envelopes();
		M2_GAME.flip_buffers();
		++gfx_count;

		if (since_last_fps.measure(); 5000 < since_last_fps.lap()) {
			since_last_fps.subtract_from_lap(5000);
			LOGF_DEBUG("PHY count %d, GFX count %d, FPS %f", phy_count, gfx_count, gfx_count / 5.0f);
			phy_count = 0;
			gfx_count = 0;
		}
	}

	Game::destroy_instance();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}
