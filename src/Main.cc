#include <m2/m3/VecF.h>
#include <m2/Proxy.h>
#include "m2/Game.h"
#include "m2/sdl/Detail.h"
#include "m2/sdl/Stopwatch.h"
#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <m2/ThreadPool.h>
#include <m2/Log.h>
#include <m2/command_line/SpriteSheets.h>

#define BREAK_IF_QUIT() if (GAME.quit) break

using namespace m2;

int main(const int argc, char **argv) {
	DEBUG_FN();

	LOG_DEBUG("Processing command line arguments...");
	for (int i = 1; i < argc; i++) {
		LOG_TRACE("Processing argument %s...", argv[i]);
		std::string arg{argv[i]};
		if (constexpr std::string_view log_level_opt = "--log-level="; arg.starts_with(log_level_opt)) {
			if (const auto opt = arg.substr(log_level_opt.size()); not LogLevel_Parse(opt, &current_log_level)) {
				LOG_WARN("Invalid log level", opt);
			}
			LOG_INFO("New log level", current_log_level);
		} else if (arg == "--silent") {
			LOG_INFO("Silent");
			silent = true;
		} else if (constexpr std::string_view slowdown_opt = "--slowdown="; arg.starts_with(slowdown_opt)) {
			auto opt = arg.substr(slowdown_opt.size());
			if (auto const slowdown_factor = strtol(opt.c_str(), nullptr, 0); 1 <= slowdown_factor) {
				time_slowdown_factor = static_cast<int>(slowdown_factor);
				LOG_INFO("New slowdown factor", time_slowdown_factor);
			} else {
				LOG_WARN("Invalid slowdown factor", opt);
			}
		} else if (arg == "--sprite-sheets") {
			LOG_INFO("Generating sprite sheets");
			printf("%s\n", generate_sprite_sheets_skeleton().c_str());
			return 0;
		} else {
			LOG_WARN("Unknown command line argument", arg);
		}
	}
	LOG_DEBUG("Processed command line arguments");

	//ThreadPool thread_pool;

	LOG_DEBUG("Initializing SDL...");
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		LOG_FATAL("SDL error", SDL_GetError());
		return -1;
	}
	LOG_DEBUG("Initialized SDL, initializing SDL_image...");
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		LOG_FATAL("SDL error", IMG_GetError());
		return -1;
	}
	LOG_DEBUG("Initialized SDL_image, initializing SDL_ttf...");
	if (TTF_Init() != 0) {
		LOG_FATAL("SDL error", TTF_GetError());
		return -1;
	}
	LOG_DEBUG("Initialized SDL_ttf");

	Game::create_instance();
	GAME.add_pause_ticks(sdl::get_ticks()); // Add initialization duration as pause ticks

	LOG_DEBUG("Executing entry UI...");
	if (m2::ui::State::create_execute_sync(PROXY.main_menu()) == m2::ui::Action::QUIT) {
		LOG_INFO("Entry UI returned QUIT");
		return 0;
	}
	LOG_DEBUG("Executed entry UI");

	sdl::Stopwatch since_last_phy(GAME.pause_ticks);
	sdl::Stopwatch since_last_gfx(GAME.pause_ticks);
	sdl::Stopwatch since_last_fps(GAME.pause_ticks);
	unsigned phy_count{}, gfx_count{}, last_phy_count = UINT_MAX;
	LOG_DEBUG("Initial ticks", GAME.pause_ticks);
	while (!GAME.quit) {
		LEVEL.begin_game_loop();

		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (last_phy_count) {
			// Clear the events only if the physics step has executed
			// Otherwise some keys/buttons may not have been handled
			GAME.events.clear();
		}
		if (GAME.events.gather()) {
			GAME.handle_quit_event();
			GAME.handle_window_resize_event();
			GAME.handle_console_event();
			GAME.handle_menu_event();
			GAME.handle_hud_events();
		}
		BREAK_IF_QUIT();
		GAME.reset_mouse_position();

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// PHYSICS ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		for (last_phy_count = 0;
			// Up to 4 times, if enough time has passed since last phy, if game hasn't quit
			last_phy_count < 4 && GAME.phy_period_ticks <= since_last_phy.measure() && !GAME.quit;
			// Increment phy counters, subtract period from stopwatch
			++last_phy_count, ++phy_count, since_last_phy.subtract_from_lap(GAME.phy_period_ticks)) {
			// Advance time by GAME.phy_period
			GAME._delta_time_s = GAME.phy_period;

			GAME.execute_pre_step();
			GAME.update_characters();
			GAME.execute_step();
			GAME.execute_post_step();
			GAME.update_sounds();
			GAME.recalculate_directional_audio();
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
		GAME._delta_time_s = static_cast<float>(since_last_gfx.last()) / 1000.0f;

		GAME.execute_pre_draw();
		GAME.update_hud_contents();
		GAME.clear_back_buffer();
		GAME.draw_background();
		GAME.draw_foreground();
		GAME.draw_lights();
		GAME.execute_post_draw();
		GAME.debug_draw();
		GAME.draw_hud();
		GAME.draw_envelopes();
		GAME.flip_buffers();
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
