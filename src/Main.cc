#include <m2/Proxy.h>
#include "m2/Game.h"
#include "m2/sdl/Detail.hh"
#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <m2/ThreadPool.h>
#include <m2/Log.h>

#define BREAK_IF_QUIT() if (GAME.quit) break

using namespace m2;

int main(int argc, char **argv) {
	DEBUG_FN();

	LOG_DEBUG("Processing command line arguments...");
	for (int i = 1; i < argc; i++) {
		LOG_TRACE("Processing argument %s...", argv[i]);
		std::string arg{argv[i]};
		if (constexpr std::string_view log_level_opt = "--log-level="; arg.starts_with(log_level_opt)) {
			auto opt = arg.substr(log_level_opt.size());
			LOG_DEBUG("Encountered log-level option", opt);
			if (not pb::LogLevel_Parse(opt, &current_log_level)) {
				LOG_WARN("Invalid log level");
			}
			LOG_INFO("New log level", current_log_level);
		} else if (arg == "--silent") {
			LOG_INFO("Silent");
			silent = true;
		} else {
			LOG_WARN("Unknown command line argument", arg);
		}
	}
	LOG_DEBUG("Processed command line arguments");

	ThreadPool tpool;

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
	if (m2::ui::execute_blocking(m2g::ui::main_menu()) == m2::ui::Action::QUIT) {
		LOG_INFO("Entry UI returned QUIT");
		return 0;
	}
	LOG_DEBUG("Executed entry UI");

	float time_since_last_phy = 0.0f;
	unsigned prev_phy_ticks = 0, prev_gfx_ticks = 0;
	unsigned prev_phy_step_count = UINT_MAX;

	unsigned frame_count_time = 0;
	unsigned phy_step_count = 0;
	unsigned gfx_draw_count = 0;
	auto frame_start_ticks = GAME.pause_ticks;
	LOG_DEBUG("Initial ticks", GAME.pause_ticks);
	while (!GAME.quit) {
		LEVEL.begin_game_loop();

		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (prev_phy_step_count) {
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
		GAME.recalculate_mouse_position();

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// PHYSICS ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		for (prev_phy_step_count = 0; prev_phy_step_count < 4 && !GAME.quit; prev_phy_step_count++) {
			auto ticks_since_prev_phy = sdl::get_ticks_since(prev_phy_ticks, GAME.pause_ticks, 1);
			prev_phy_ticks += ticks_since_prev_phy;
			time_since_last_phy += (float)ticks_since_prev_phy / 1000.0f;
			if (time_since_last_phy <= GAME.phy_period) {
				break;
			}

			// Advance time by GAME.phy_period
			GAME._delta_time_s = GAME.phy_period;

			GAME.execute_pre_step();
			BREAK_IF_QUIT();
			GAME.update_characters();
			BREAK_IF_QUIT();
			GAME.execute_step();
			BREAK_IF_QUIT();
			GAME.execute_post_step();
			BREAK_IF_QUIT();
			GAME.update_sounds();
			BREAK_IF_QUIT();
			GAME.recalculate_directional_audio();

			++phy_step_count;
			time_since_last_phy -= GAME.phy_period;
		}
		if (prev_phy_step_count == 4) {
			time_since_last_phy = 0.0f;
		}
		BREAK_IF_QUIT();

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		auto delta_ticks = sdl::get_ticks_since(prev_gfx_ticks, GAME.pause_ticks, 1);
		GAME._delta_time_s = static_cast<float>(delta_ticks) / 1000.0f;
		prev_gfx_ticks += delta_ticks;


		GAME.execute_pre_draw();
		GAME.update_hud_contents();
		GAME.clear_back_buffer();
        GAME.draw_background();
		GAME.draw_foreground();
        GAME.draw_lights();
		GAME.draw_background_effects();
		GAME.draw_foreground_effects();
		GAME.execute_post_draw();
		GAME.draw_debug_shapes();
		GAME.draw_hud();
		GAME.draw_envelopes();
		GAME.flip_buffers();

		++gfx_draw_count;

		auto frame_end_ticks = sdl::get_ticks();
		frame_count_time += frame_end_ticks - frame_start_ticks;
		frame_start_ticks = frame_end_ticks;
		if (1000 < frame_count_time) {
			frame_count_time -= 1000;
			LOGF_TRACE("PHY count %d, GFX count %d", phy_step_count, gfx_draw_count);
			phy_step_count = 0;
			gfx_draw_count = 0;
		}
	}

	Game::destroy_instance();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}
