#include <m2/m3/VecF.h>
#include <m2/Proxy.h>
#include "m2/Game.h"
#include "m2/sdl/Detail.h"
#include "m2/sdl/Stopwatch.h"
#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <m2/Log.h>

#define BREAK_IF_QUIT() if (M2_GAME.quit) break

using namespace m2;

int main(const int argc, char **argv) {
	StoreInitialStackPosition();
	SetThreadNameForLogging("MN");
	INFO_FN();

	if (auto success = load_options(argc, argv); not success) {
		LOG_ERROR("Error while loading program arguments", success.error());
		return -1;
	}

	Game::CreateInstance();

	std::optional<sdl::Stopwatch> sinceLastPhy;
	std::optional<Stopwatch> prevGfxUpdateAt, prevFpsLogAt;
	unsigned phy_count{}, gfx_count{}, last_phy_count = UINT_MAX;
	while (not M2_GAME.quit) {
		// If the level is marked for deletion, delete it
		if (M2_GAME.HasLevel() && M2_LEVEL.IsMarkedForDeletion()) {
			M2_GAME.UnloadLevel();
		}

		// Try to load a level if there's no level
		if (not M2_GAME.HasLevel()) {
			prevGfxUpdateAt.reset();

			LOG_DEBUG("Executing main menu...");
			if (UiPanel::create_and_run_blocking(M2G_PROXY.MainMenuBlueprint()).IsQuit()) {
				LOG_INFO("Main menu returned QUIT");
				break;
			}
			if (not M2_GAME.HasLevel()) {
				LOG_WARN("Main menu didn't initialize a level");
				continue;
			}
			LOG_INFO("Main menu loaded a level");

			M2_LEVEL.BeginGameLoop();
			sinceLastPhy = sdl::Stopwatch{};
			prevGfxUpdateAt = Stopwatch{}; // Act as-if a graphics update was just done
			prevFpsLogAt = Stopwatch{}; // Act as-if FPS log was just done
		}

		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (last_phy_count) {
			// Clear the events only if the physics step has executed
			// Otherwise some keys/buttons may not have been handled
			M2_GAME.events.Clear();
		}
		if (M2_GAME.events.Gather()) {
			M2_GAME.HandleQuitEvent();
			M2_GAME.HandleWindowResizeEvent();
			M2_GAME.HandleConsoleEvent();
			M2_GAME.HandlePauseEvent();
			M2_GAME.HandleHudEvents();
			M2_GAME.HandleNetworkEvents();
			M2_GAME.ExecuteDeferredActions();
		}
		BREAK_IF_QUIT();
		M2_GAME.ResetMousePosition();

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// PHYSICS ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		last_phy_count = 0;
		// Up to 4 times
		m2Repeat(4) {
			BREAK_IF_QUIT();

			sinceLastPhy->measure(M2_LEVEL.GetTotalPauseDurationMsTMP());
			const auto enoughTimeHasPassed = TIME_BETWEEN_PHYSICS_SIMULATIONS_MS <= sinceLastPhy->duration_of_lap();
			if (not enoughTimeHasPassed) {
				break;
			}

			M2_GAME._delta_time_s = TIME_BETWEEN_PHYSICS_SIMULATIONS_F;
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

			// Increment phy counters, subtract period from stopwatch
			++last_phy_count;
			++phy_count;
			sinceLastPhy->subtract_from_lap(TIME_BETWEEN_PHYSICS_SIMULATIONS_MS);
		}
		if (last_phy_count == 4) {
			sinceLastPhy->new_lap();
		}
		BREAK_IF_QUIT();

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Measure and advance time
		const auto durationSinceLastGfx = prevGfxUpdateAt->Reset();
		M2_GAME._delta_time_s = std::chrono::duration_cast<std::chrono::duration<float>>(durationSinceLastGfx).count();

		M2_GAME.ExecutePreDraw();
		M2_GAME.UpdateHudContents();
		M2_GAME.ClearBackBuffer();
		M2_GAME.Draw();
		M2_GAME.DrawLights();
		M2_GAME.ExecutePostDraw();
		M2_GAME.DebugDraw();
		M2_GAME.DrawHud();
		M2_GAME.DrawEnvelopes();
		M2_GAME.FlipBuffers();
		++gfx_count;

		if (prevFpsLogAt->HasTimePassed(TIME_BETWEEN_FPS_LOGS)) {
			prevFpsLogAt->AdvanceStartTimePoint(TIME_BETWEEN_FPS_LOGS);
			LOGF_DEBUG("PHY count %d, GFX count %d, FPS %f", phy_count, gfx_count, gfx_count / 10.0f);
			phy_count = 0;
			gfx_count = 0;
		}
	}

	Game::DestroyInstance();
	return 0;
}
