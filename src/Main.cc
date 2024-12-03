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
	set_thread_name_for_logging("MN");
	INFO_FN();

	auto arg_list = to_argument_list(argc, argv);
	if (auto log_level_opt = parse_argument(arg_list, "log-level")) {
		if (not LogLevel_Parse(*log_level_opt, &current_log_level)) {
			LOG_WARN("Invalid log level", *log_level_opt);
		}
		LOG_INFO("New log level", current_log_level);
	}
	if (auto silent_opt = parse_argument(arg_list, "silent")) {
		LOG_INFO("Silent");
		silent = true;
	}
	if (auto slowdown_opt = parse_argument(arg_list, "slowdown")) {
		if (auto const slowdown_factor = strtol(slowdown_opt->c_str(), nullptr, 0); 1 <= slowdown_factor) {
			time_slowdown_factor = static_cast<int>(slowdown_factor);
			LOG_INFO("New slowdown factor", time_slowdown_factor);
		} else {
			LOG_WARN("Invalid slowdown factor", *slowdown_opt);
		}
	}
	if (auto sprite_sheets_opt = parse_argument(arg_list, "sprite-sheets")) {
		LOG_INFO("Generating sprite sheets");
		printf("%s\n", generate_sprite_sheets_skeleton().c_str());
		return 0;
	}
	if (auto console_opt = parse_argument(arg_list, "console")) {
		console_command = *console_opt;
		LOG_INFO("Console command", *console_opt);
	}
	if (auto god_mode_opt = parse_argument(arg_list, "god-mode")) {
		LOG_INFO("God mode");
		god_mode = true;
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

	Game::CreateInstance();
	M2_GAME.AddPauseTicks(sdl::get_ticks()); // Add initialization duration as pause ticks

	LOG_DEBUG("Executing main menu...");
	if (m2::ui::Panel::create_and_run_blocking(M2G_PROXY.main_menu()).is_quit()) {
		LOG_INFO("Main menu returned QUIT");
		return 0;
	}
	LOG_DEBUG("Main menu returned");

	// Check if there's a level
	if (not M2_GAME.HasLevel()) {
		LOG_WARN("Main menu didn't initialize a level");
	}

	sdl::Stopwatch since_last_phy(M2_GAME.pause_ticks);
	sdl::Stopwatch since_last_gfx(M2_GAME.pause_ticks);
	sdl::Stopwatch since_last_fps(M2_GAME.pause_ticks);
	unsigned phy_count{}, gfx_count{}, last_phy_count = UINT_MAX;
	LOG_DEBUG("Initial pause ticks", M2_GAME.pause_ticks);
	while (M2_GAME.HasLevel() && !M2_GAME.quit) {
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
			M2_GAME.HandleQuitEvent();
			M2_GAME.HandleWindowResizeEvent();
			M2_GAME.HandleConsoleEvent();
			M2_GAME.HandleMenuEvent();
			M2_GAME.HandleHudEvents();
			M2_GAME.HandleNetworkEvents();
			M2_GAME.ExecuteDeferredActions();
		}
		BREAK_IF_QUIT();
		M2_GAME.ResetMousePosition();

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

			M2_GAME.ExecutePreStep();
			M2_GAME.ExecuteDeferredActions();
			M2_GAME.UpdateCharacters();
			M2_GAME.ExecuteDeferredActions();
			M2_GAME.ExecuteStep();
			M2_GAME.ExecuteDeferredActions();
			M2_GAME.ExecutePostStep();
			M2_GAME.ExecuteDeferredActions();
			M2_GAME.UpdateSounds();
			M2_GAME.ExecuteDeferredActions();
			M2_GAME.RecalculateDirectionalAudio();
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

		M2_GAME.ExecutePreDraw();
		M2_GAME.UpdateHudContents();
		M2_GAME.ClearBackBuffer();
		M2_GAME.DrawBackground();
		M2_GAME.DrawForeground();
		M2_GAME.DrawLights();
		M2_GAME.ExecutePostDraw();
		M2_GAME.DebugDraw();
		M2_GAME.DrawHud();
		M2_GAME.DrawEnvelopes();
		M2_GAME.FlipBuffers();
		++gfx_count;

		if (since_last_fps.measure(); 5000 < since_last_fps.lap()) {
			since_last_fps.subtract_from_lap(5000);
			LOGF_DEBUG("PHY count %d, GFX count %d, FPS %f", phy_count, gfx_count, gfx_count / 5.0f);
			phy_count = 0;
			gfx_count = 0;
		}
	}

	Game::DestroyInstance();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}
