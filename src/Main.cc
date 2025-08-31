#include <m2/m3/VecF.h>
#include <m2/Proxy.h>
#include "m2/Game.h"
#include "m2/sdl/Detail.h"
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

	// Used to keep track of when the physics update was last executed. This stopwatch is usually not reset after each
	// simulation, but its starting point is advanced by physics simulation period, thus the duration will contain the
	// total pause duration since the beginning of the level, and it must be subtracked before use. Rarely, the
	// stopwatch could be reset if the engine can't keep up with real time. In this case, the starting point of the
	// stopwatch will be retrated by the total pause duration to keep the calculations simple.
	std::optional<Stopwatch> prevPhyUpdateAt;
	// Used to keep track of when the graphics update was last executed. This stopwatch is reset after every execution,
	// thus each measurement might contain the pause duration since the last execution. To find the pause duration since
	// the last execution, total pause duration during last update and current update are compared.
	std::optional<Stopwatch> prevGfxUpdateAt;
	std::optional<Stopwatch::Duration> prevTotalPauseDurationDuringLastGfx;
	// Used to keep track of when the FPS log was last printed. This stopwatch is never reset, but instead its starting
	// point is advanced by log period after each execution, thus the duration will contain the total pause duration
	// since the beginning of the level, and it must be subtracked before use.
	std::optional<Stopwatch> prevFpsLogAt;

	unsigned totalPhySimulationCount{}, totalGfxUpdateCount{}, prevPhySimulationCount = UINT_MAX;

	while (not M2_GAME.quit) {
		// If the level is marked for deletion, delete it
		if (M2_GAME.HasLevel() && M2_LEVEL.IsMarkedForDeletion()) {
			M2_GAME.UnloadLevel();
		}

		// Try to load a level if there's no level
		if (not M2_GAME.HasLevel()) {
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
			prevPhyUpdateAt = Stopwatch{}; // Act as-if a physics update was just done
			prevGfxUpdateAt = Stopwatch{}; // Act as-if a graphics update was just done
			prevTotalPauseDurationDuringLastGfx = Stopwatch::Duration{};
			prevFpsLogAt = Stopwatch{}; // Act as-if FPS log was just done
		}

		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (prevPhySimulationCount) {
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
		prevPhySimulationCount = 0;
		m2Repeat(4) { // Up to 4 times
			BREAK_IF_QUIT();

			if (prevPhyUpdateAt->GetDurationSince() - M2_LEVEL.GetTotalPauseDuration() < TIME_BETWEEN_PHYSICS_SIMULATIONS) {
				break;
			}
			prevPhyUpdateAt->AdvanceStartingPoint(TIME_BETWEEN_PHYSICS_SIMULATIONS);

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
			++prevPhySimulationCount;
			++totalPhySimulationCount;
		}
		if (prevPhySimulationCount == 4) {
			LOG_WARN("Physics simulation can't keep up with wall clock, simulation will jump forward");
			prevPhyUpdateAt->Reset();
			prevPhyUpdateAt->RetracktStartingPoint(M2_LEVEL.GetTotalPauseDuration());
		}
		BREAK_IF_QUIT();

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////

		const auto currentTotalPauseDuration = M2_LEVEL.GetTotalPauseDuration();
		const auto pauseDurationSinceLastGfx = currentTotalPauseDuration - *prevTotalPauseDurationDuringLastGfx;
		const auto ingameDurationSinceLastGfx = prevGfxUpdateAt->Reset() - pauseDurationSinceLastGfx;
		M2_GAME._delta_time_s = std::chrono::duration_cast<std::chrono::duration<float>>(ingameDurationSinceLastGfx).count();
		prevTotalPauseDurationDuringLastGfx = currentTotalPauseDuration;

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
		++totalGfxUpdateCount;

		if (TIME_BETWEEN_FPS_LOGS <= prevFpsLogAt->GetDurationSince() - M2_LEVEL.GetTotalPauseDuration()) {
			prevFpsLogAt->AdvanceStartingPoint(TIME_BETWEEN_FPS_LOGS);
			LOGF_DEBUG("PHY count %d, GFX count %d, FPS %f", totalPhySimulationCount, totalGfxUpdateCount, totalGfxUpdateCount / 10.0f);
			totalPhySimulationCount = 0;
			totalGfxUpdateCount = 0;
		}
	}

	Game::DestroyInstance();
	return 0;
}
