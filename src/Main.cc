#include <m2g/Proxy.h>
#include "m2/Events.h"
#include "m2/Game.hh"
#include "m2/sdl/Utils.hh"
#include <m2g/Ui.h>
#include <SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <m2/ThreadPool.h>
#include <m2/Log.h>
#include <m2/Map2i.h>
#include <cstdlib>

using namespace m2;

//void AudioCallback(void* userdata, uint8_t* stream, int len) {
//	for (int i = 0; i < len; i++) {
//		stream[i] = (i % 384) < 192 ? 127 : -128;
//	}
//}

int main(int argc, char **argv) {
	DEBUG_FN();

	if (b2_version.major != 2 || b2_version.minor != 4 || b2_version.revision != 0) {
		throw M2FATAL("Box2D version mismatch");
	}

	// Process command line arguments
	for (int i = 1; i < argc; i++) {
		const char* loglevel = "--log-lvl=";
		const size_t loglevelStrlen = strlen(loglevel);
		if (strncmp(argv[i], loglevel, loglevelStrlen) == 0) {
			if (strcmp(argv[i] + loglevelStrlen, "trace") == 0) {
				m2::current_log_level = m2::LogLevel::Trace;
			} else if (strcmp(argv[i] + loglevelStrlen, "debug") == 0) {
				m2::current_log_level = m2::LogLevel::Debug;
			} else if (strcmp(argv[i] + loglevelStrlen, "info") == 0) {
				m2::current_log_level = m2::LogLevel::Info;
			} else if (strcmp(argv[i] + loglevelStrlen, "warning") == 0) {
				m2::current_log_level = m2::LogLevel::Warn;
			} else if (strcmp(argv[i] + loglevelStrlen, "error") == 0) {
				m2::current_log_level = m2::LogLevel::Error;
			} else if (strcmp(argv[i] + loglevelStrlen, "fatal") == 0) {
				m2::current_log_level = m2::LogLevel::Fatal;
			} else {
				LOG_WARN("Invalid log level");
			}
			LOG_INFO("Current log level", m2::current_log_level);
		} else {
			LOG_WARN("Invalid command line argument");
		}
	}

	ThreadPool tpool;

	// Global initialization
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		LOG_FATAL("SDL error", SDL_GetError());
		return -1;
	}
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		LOG_FATAL("SDL error", IMG_GetError());
		return -1;
	}
	if (TTF_Init() != 0) {
		LOG_FATAL("SDL error", TTF_GetError());
		return -1;
	}

	Game::create_instance();

//	int audioDriverCount = SDL_GetNumAudioDrivers();
//	fprintf(stderr, "SDL_GetNumAudioDrivers: %d\n", audioDriverCount);
//	for (int i = 0; i < audioDriverCount; i++) {
//		fprintf(stderr, "AudioDriver: %s\n", SDL_GetAudioDriver(i));
//	}
//	fprintf(stderr, "SDL_GetCurrentAudioDriver: %s\n", SDL_GetCurrentAudioDriver());
//
//	int audioDeviceCount = SDL_GetNumAudioDevices(0);
//	fprintf(stderr, "SDL_GetNumAudioDevices: %d\n", audioDeviceCount);
//	for (int i = 0; i < audioDeviceCount; i++) {
//		fprintf(stderr, "AudioDevice: %s\n", SDL_GetAudioDeviceName(i, 0));
//	}

//	SDL_AudioSpec want, have;
//	SDL_memset(&want, 0, sizeof(want));
//	want.freq = 48000;
//	want.format = AUDIO_S8;
//	want.channels = 1;
//	want.samples = 4096;
//	want.callback = AudioCallback;
//	SDL_AudioDeviceID audioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
//	if (audioDeviceId == 0) {
//		fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
//	} else {
//		fprintf(stderr, "want.freq=%d have.freq=%d\n", want.freq, have.freq);
//		fprintf(stderr, "want.format=%d have.format=%d\n", want.format, have.format);
//		fprintf(stderr, "want.channels=%d have.channels=%d\n", want.channels, have.channels);
//		fprintf(stderr, "want.samples=%d have.samples=%d\n", want.samples, have.samples);
//		SDL_PauseAudioDevice(audioDeviceId, 0);
//		SDL_Delay(5000);
//		SDL_PauseAudioDevice(audioDeviceId, 1);
//	}

	if (m2::ui::execute_blocking(&m2g::ui::entry) == m2::ui::Action::QUIT) {
		return 0;
	}

	float timeSinceLastWorldStep = 0.0f;
	auto nongame_ticks = SDL_GetTicks();
	unsigned prevPrePhysicsTicks = 0, prevWorldStepTicks = 0, prevPostPhysicsTicks = 0, prevTerrainDrawGraphicsTicks = 0,
		prevPreGraphicsTicks = 0, prevDrawTicks = 0, prevDrawLightsTicks = 0, prevPostGraphicsTicks = 0;

	unsigned frameTimeAccumulator = 0;
	unsigned frameCount = 0;
	unsigned phy_step_count = UINT_MAX;
	while (true) {
		unsigned start_ticks = SDL_GetTicks();

		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (phy_step_count) {
			// Clear the events only if the physics step has executed
			// Otherwise some keys/buttons may not have been handled
			GAME.events.clear();
		}
		if (GAME.events.gather()) {
			// Handle quit event
			if (GAME.events.pop_quit()) {
				break;
			}
			// Handle window resize event
			auto window_resize = GAME.events.pop_window_resize();
			if (window_resize) {
				GAME.update_window_dims(window_resize->x, window_resize->y);
				if (GAME.leftHudUIState) {
					GAME.leftHudUIState->update_positions(GAME.leftHudRect);
				}
                if (GAME.rightHudUIState) {
	                GAME.rightHudUIState->update_positions(GAME.rightHudRect);
				}
			}
            // Handle key events
            if (GAME.events.pop_key_press(Key::MENU)) {
                uint32_t pause_start_ticks = SDL_GetTicks();
                if (m2::ui::execute_blocking(&m2g::ui::pause) == m2::ui::Action::QUIT) {
                    return 0;
                }
                uint32_t pause_end_ticks = SDL_GetTicks();
                nongame_ticks += pause_end_ticks - pause_start_ticks;
            }
            if (GAME.events.pop_key_press(Key::CONSOLE)) {
                uint32_t pause_start_ticks = SDL_GetTicks();
                if (m2::ui::execute_blocking(&m2::ui::console_ui) == m2::ui::Action::QUIT) {
                    return 0;
                }
                uint32_t pause_end_ticks = SDL_GetTicks();
                nongame_ticks += pause_end_ticks - pause_start_ticks;
            }
            // Handle HUD events (mouse and key)
			IF(GAME.leftHudUIState)->handle_events(GAME.events);
			IF(GAME.rightHudUIState)->handle_events(GAME.events);
		}
		GAME.update_mouse_position();
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// PHYSICS ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		for (phy_step_count = 0; phy_step_count < 4; phy_step_count++) {
			uint32_t ticksSinceLastWorldStep =
					SdlUtils_GetTicksAtLeast1ms(prevWorldStepTicks, nongame_ticks) - prevWorldStepTicks;
			prevWorldStepTicks += ticksSinceLastWorldStep;
			timeSinceLastWorldStep += (float)ticksSinceLastWorldStep / 1000.0f;
			if (GAME.physicsStep_s < timeSinceLastWorldStep) {
				// Pre-physics
				GAME.deltaTicks_ms =
						SdlUtils_GetTicksAtLeast1ms(prevPrePhysicsTicks, nongame_ticks) - prevPrePhysicsTicks;
				GAME.deltaTime_s = (float) GAME.deltaTicks_ms / 1000.0f;
				prevPrePhysicsTicks += GAME.deltaTicks_ms;
				for (auto monitor_it: GAME.monitors) {
					IF(monitor_it.first->pre_phy)(*monitor_it.first);
				}
				GAME.execute_deferred_actions();

				// Physics
				if (GAME.world) {
					GAME.is_phy_stepping = true; // TODO replace with b2World->IsLocked()
					GAME.world->Step(GAME.physicsStep_s, GAME.velocityIterations, GAME.positionIterations);
					GAME.is_phy_stepping = false;
					// Update positions
					for (auto physique_it : GAME.physics) {
						auto object_id = physique_it.first->object_id;
						auto& object = GAME.objects[object_id];
						auto old_pos = object.position;
						object.position = m2::Vec2f{physique_it.first->body->GetPosition()};
						if (old_pos != object.position) {
							GAME.draw_list.queue_update(object_id, object.position);
						}
					}
				}
				GAME.draw_list.update();

				// Post-physics
				GAME.deltaTicks_ms =
						SdlUtils_GetTicksAtLeast1ms(prevPostPhysicsTicks, nongame_ticks) - prevPostPhysicsTicks;
				GAME.deltaTime_s = (float) GAME.deltaTicks_ms / 1000.0f;
				prevPostPhysicsTicks += GAME.deltaTicks_ms;
				for (auto monitor_it: GAME.monitors) {
					IF(monitor_it.first->post_phy)(*monitor_it.first);
				}
				GAME.execute_deferred_actions();

				// Update loop condition
				timeSinceLastWorldStep -= GAME.physicsStep_s;
			} else {
				break;
			}
		}
		if (phy_step_count == 4) {
			timeSinceLastWorldStep = 0.0f;
		}
		//////////////////////////// END OF PHYSICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Pre-graphic
		GAME.deltaTicks_ms = SdlUtils_GetTicksAtLeast1ms(prevPreGraphicsTicks, nongame_ticks) - prevPreGraphicsTicks;
		GAME.deltaTime_s = (float)GAME.deltaTicks_ms / 1000.0f;
		prevPreGraphicsTicks += GAME.deltaTicks_ms;
		for (auto monitor_it : GAME.monitors) {
			IF(monitor_it.first->pre_gfx)(*monitor_it.first);
		}

		// HUD
		IF(GAME.leftHudUIState)->update_contents();
		IF(GAME.rightHudUIState)->update_contents();

		// Clear screen
		SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
		SDL_RenderClear(GAME.sdlRenderer);

		// Draw terrain
		GAME.deltaTicks_ms =
				SdlUtils_GetTicksAtLeast1ms(prevTerrainDrawGraphicsTicks, nongame_ticks) - prevTerrainDrawGraphicsTicks;
		GAME.deltaTime_s = (float)GAME.deltaTicks_ms / 1000.0f;
		prevTerrainDrawGraphicsTicks += GAME.deltaTicks_ms;
        for (auto graphic_it : GAME.terrainGraphics) {
			IF(graphic_it.first->on_draw)(*graphic_it.first);
        }

		// Draw
		GAME.deltaTicks_ms = SdlUtils_GetTicksAtLeast1ms(prevDrawTicks, nongame_ticks) - prevDrawTicks;
		GAME.deltaTime_s = (float)GAME.deltaTicks_ms / 1000.0f;
		prevDrawTicks += GAME.deltaTicks_ms;
		for (const auto& gfx_id : GAME.draw_list) {
			auto& gfx = GAME.graphics[gfx_id];
			IF(gfx.on_draw)(gfx);
		}

		// Draw lights
		GAME.deltaTicks_ms = SdlUtils_GetTicksAtLeast1ms(prevDrawLightsTicks, nongame_ticks) - prevDrawLightsTicks;
		GAME.deltaTime_s = (float)GAME.deltaTicks_ms / 1000.0f;
		prevDrawLightsTicks += GAME.deltaTicks_ms;
        for (auto light_it : GAME.lights) {
			IF(light_it.first->on_draw)(*light_it.first);
        }

#ifdef DEBUG
		// Draw debug shapes
		for (auto physique_it : GAME.physics) {
			physique_it.first->draw_shapes();
		}
#endif

		// HUD
		IF(GAME.leftHudUIState)->draw();
		IF(GAME.rightHudUIState)->draw();

		// Draw envelope
		SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
		SDL_RenderFillRect(GAME.sdlRenderer, &GAME.topEnvelopeRect);
		SDL_RenderFillRect(GAME.sdlRenderer, &GAME.bottomEnvelopeRect);
		SDL_RenderFillRect(GAME.sdlRenderer, &GAME.leftEnvelopeRect);
		SDL_RenderFillRect(GAME.sdlRenderer, &GAME.rightEnvelopeRect);

		// Present
		SDL_RenderPresent(GAME.sdlRenderer);

		// Post-graphic
		GAME.deltaTicks_ms = SdlUtils_GetTicksAtLeast1ms(prevPostGraphicsTicks, nongame_ticks) - prevPostGraphicsTicks;
		GAME.deltaTime_s = (float)GAME.deltaTicks_ms / 1000.0f;
		prevPostGraphicsTicks += GAME.deltaTicks_ms;
		for (auto monitor_it : GAME.monitors) {
			IF(monitor_it.first->post_gfx)(*monitor_it.first);
		}
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////

		unsigned end_ticks = SDL_GetTicks();
		frameTimeAccumulator += end_ticks - start_ticks;
		frameCount++;
		if (1000 < frameTimeAccumulator) {
			frameTimeAccumulator -= 1000;
			LOGF_TRACE("FPS %d", frameCount);
			frameCount = 0;
		}
	}

	Game::destroy_instance();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}
