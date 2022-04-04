#define _CRT_SECURE_NO_WARNINGS
#include <b2_world.h>
#include "m2/Event.hh"
#include "m2/Game.hh"
#include "m2/SDLUtils.hh"
#include "m2/Def.hh"
#include "m2/Cfg.hh"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdlib.h>

//void AudioCallback(void* userdata, uint8_t* stream, int len) {
//	for (int i = 0; i < len; i++) {
//		stream[i] = (i % 384) < 192 ? 127 : -128;
//	}
//}

int main(int argc, char **argv) {
	LOG_DEBUG_FN();

	// Process command line arguments
	for (int i = 1; i < argc; i++) {
		const char* loglevel = "--log-level=";
		const size_t loglevelStrlen = strlen(loglevel);
		if (strncmp(argv[i], loglevel, loglevelStrlen) == 0) {
			if (strcmp(argv[i] + loglevelStrlen, "trace") == 0) {
				gCurrentLogLevel = LogLevelTrace;
				LOG_INFO_M2V(M2_LOG_LEVEL, Int32, LogLevelTrace);
			} else if (strcmp(argv[i] + loglevelStrlen, "debug") == 0) {
				gCurrentLogLevel = LogLevelDebug;
				LOG_INFO_M2V(M2_LOG_LEVEL, Int32, LogLevelDebug);
			} else if (strcmp(argv[i] + loglevelStrlen, "info") == 0) {
				gCurrentLogLevel = LogLevelInfo;
				LOG_INFO_M2V(M2_LOG_LEVEL, Int32, LogLevelInfo);
			} else if (strcmp(argv[i] + loglevelStrlen, "warning") == 0) {
				gCurrentLogLevel = LogLevelWarn;
				LOG_INFO_M2V(M2_LOG_LEVEL, Int32, LogLevelWarn);
			} else if (strcmp(argv[i] + loglevelStrlen, "error") == 0) {
				gCurrentLogLevel = LogLevelError;
				LOG_INFO_M2V(M2_LOG_LEVEL, Int32, LogLevelError);
			} else if (strcmp(argv[i] + loglevelStrlen, "fatal") == 0) {
				gCurrentLogLevel = LogLevelFatal;
				LOG_INFO_M2V(M2_LOG_LEVEL, Int32, LogLevelFatal);
			} else {
				LOG_WARNING("Invalid log level");
			}
		} else {
			LOG_WARNING("Invalid command line argument");
		}
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, SDL_GetError());
		return -1;
	}
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, IMG_GetError());
		return -1;
	}
	if (TTF_Init() != 0) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, TTF_GetError());
		return -1;
	}
	Game_UpdateWindowDimensions(1600, 900); // Store default window dimensions in GAME
	if ((GAME.sdlWindow = SDL_CreateWindow("m2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GAME.windowRect.w, GAME.windowRect.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, SDL_GetError());
		return -1;
	}
	SDL_SetWindowMinimumSize(GAME.sdlWindow, 712, 400);
	SDL_StopTextInput(); // Text input begins activated (sometimes)
	GAME.sdlCursor = SDLUtils_CreateCursor();
	SDL_SetCursor(GAME.sdlCursor);
	if ((GAME.pixelFormat = SDL_GetWindowPixelFormat(GAME.sdlWindow)) == SDL_PIXELFORMAT_UNKNOWN) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, SDL_GetError());
		return -1;
	}
	if ((GAME.sdlRenderer = SDL_CreateRenderer(GAME.sdlWindow, -1, SDL_RENDERER_ACCELERATED)) == NULL) { // SDL_RENDERER_PRESENTVSYNC
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, SDL_GetError());
		return -1;
	}
//	SDL_Surface* textureMapSurface = IMG_Load(CFG_TEXTURE_FILE);
//	if (textureMapSurface == NULL) {
//		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, IMG_GetError());
//		return -1;
//	}
//	if ((GAME.sdlTexture = SDL_CreateTextureFromSurface(GAME.sdlRenderer, textureMapSurface)) == NULL) {
//		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, SDL_GetError());
//		return -1;
//	}
//	SDL_SetTextureColorMod(GAME.sdlTexture, 127, 127, 127);
//	SDL_FreeSurface(textureMapSurface);
//	SDL_Surface* textureMaskSurface = IMG_Load(CFG_TEXTURE_MASK_FILE);
//	if (textureMaskSurface == NULL) {
//		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, IMG_GetError());
//		return -1;
//	}
//	if ((GAME.sdlTextureMask = SDL_CreateTextureFromSurface(GAME.sdlRenderer, textureMaskSurface)) == NULL) {
//		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, SDL_GetError());
//		return -1;
//	}
//	SDL_FreeSurface(textureMaskSurface);
	SDL_Surface* lightSurface = IMG_Load("resource/RadialGradient-WhiteBlack.png");
	if (lightSurface == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, IMG_GetError());
		return -1;
	}
	if ((GAME.sdlLightTexture = SDL_CreateTextureFromSurface(GAME.sdlRenderer, lightSurface)) == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, SDL_GetError());
		return -1;
	}
	SDL_FreeSurface(lightSurface);
	SDL_SetTextureBlendMode(GAME.sdlLightTexture, SDL_BLENDMODE_MUL);
	SDL_SetTextureAlphaMod(GAME.sdlLightTexture, 0);
	SDL_SetTextureColorMod(GAME.sdlLightTexture, 127, 127, 127);
	if ((GAME.ttfFont = TTF_OpenFont("resource/fonts/joystix/joystix-monospace.ttf", 16)) == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, CString, TTF_GetError());
		return -1;
	}

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
//	SDL_AudioDeviceID audioDeviceId = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
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

    GAME.proxy.activate();
    M2Err entry_ui_result = GAME.proxy.exec_entry_ui();
    if (entry_ui_result == M2ERR_QUIT) {
        return 0;
    } else if (entry_ui_result) {
        return 1;
    }

	float timeSinceLastWorldStep = 0.0f;
	unsigned prevPrePhysicsTicks = SDL_GetTicks();
	unsigned prevWorldStepTicks = SDL_GetTicks();
	unsigned prevPostPhysicsTicks = SDL_GetTicks();
	unsigned prevTerrainDrawGraphicsTicks = SDL_GetTicks();
	unsigned prevPreGraphicsTicks = SDL_GetTicks();
	unsigned prevDrawTicks = SDL_GetTicks();
	unsigned prevDrawLightsTicks = SDL_GetTicks();
	unsigned prevPostGraphicsTicks = SDL_GetTicks();
	SDL_Delay(1); // Make sure SDL_GetTicks() doesn't return 0

	unsigned frameTimeAccumulator = 0;
	unsigned frameCount = 0;
	while (true) {
		unsigned start_ticks = SDL_GetTicks();

		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (Events_Gather(&GAME.events)) {
			// Handle quit event
			if (GAME.events.quitEvent) { break; }
			// Handle window resize event
			if (GAME.events.windowResizeEvent) {
				Game_UpdateWindowDimensions(GAME.events.windowDims.x, GAME.events.windowDims.y);
                GAME.leftHudUIState.update_positions(GAME.leftHudRect);
                GAME.rightHudUIState.update_positions(GAME.rightHudRect);
			}
			if (!SDL_IsTextInputActive()) {
				// Handle key events
				if (GAME.events.keysPressed[KEY_MENU]) {
					M2Err result = GAME.proxy.exec_pause_ui();
					if (result == M2ERR_QUIT) {
						return 0;
					} else if (result) {
						return 1;
					}
				}
				if (GAME.events.keysPressed[KEY_CONSOLE]) {
					memset(GAME.consoleInput, 0, sizeof(GAME.consoleInput));
					SDL_StartTextInput();
					LOG_INFO("SDL text input activated");
				}
				// Handle HUD events (mouse and key)
                auto left_hud_pressed_button = GAME.leftHudUIState.handle_events(GAME.events);
                if (left_hud_pressed_button) {
                    // There are no hud buttons yet that we care about
                }
                auto right_hud_pressed_button = GAME.rightHudUIState.handle_events(GAME.events);
                if (right_hud_pressed_button) {
                    // There are no hud buttons yet that we care about
                }
			} else {
				// Handle text input
				if (GAME.events.keysPressed[KEY_MENU]) {
					SDL_StopTextInput();
					LOG_INFO("SDL text input deactivated");
				} else if (GAME.events.keysPressed[KEY_ENTER]) {
					// TODO Execute console command
					LOG_INFO("Console command");
					LOG_INFO(GAME.consoleInput);
					SDL_StopTextInput();
					LOG_INFO("SDL text input deactivated");
				} else if (GAME.events.textInputEvent) {
					strcat(GAME.consoleInput, GAME.events.textInput);
					LOG_INFO("Console buffer");
					LOG_INFO(GAME.consoleInput);
				}
			}
		}
		Game_UpdateMousePosition();
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// PHYSICS ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		uint32_t ticksSinceLastWorldStep = SDLUtils_GetTicksAtLeast1ms(prevWorldStepTicks) - prevWorldStepTicks;
		prevWorldStepTicks += ticksSinceLastWorldStep;
		timeSinceLastWorldStep += ticksSinceLastWorldStep / 1000.0f;
		while (GAME.physicsStep_s < timeSinceLastWorldStep) {
			// Pre-physics
			GAME.deltaTicks_ms = SDLUtils_GetTicksAtLeast1ms(prevPrePhysicsTicks) - prevPrePhysicsTicks;
			GAME.deltaTime_s = GAME.deltaTicks_ms / 1000.0f;
			prevPrePhysicsTicks += GAME.deltaTicks_ms;
            for (auto monitor_it : GAME.monitors) {
                if (monitor_it.first->prePhysics) {
                    monitor_it.first->prePhysics(*monitor_it.first);
                }
            } // TODO Hard to parallelize
			Game_DeleteList_DeleteAll();

			// Physics
			GAME.world->Step(GAME.physicsStep_s, GAME.velocityIterations, GAME.positionIterations);
            for (auto physique_it : GAME.physics) {
				GAME.objects[physique_it.first->object_id].position = m2::Vec2f{physique_it.first->body->GetPosition()};
            } // TODO Easy to parallelize
			Game_DeleteList_DeleteAll();

			// Post-physics
			GAME.deltaTicks_ms = SDLUtils_GetTicksAtLeast1ms(prevPostPhysicsTicks) - prevPostPhysicsTicks;
			GAME.deltaTime_s = GAME.deltaTicks_ms / 1000.0f;
			prevPostPhysicsTicks += GAME.deltaTicks_ms;
            for (auto monitor_it : GAME.monitors) {
                if (monitor_it.first->postPhysics) {
                    monitor_it.first->postPhysics(*monitor_it.first);
                }
            } // TODO Hard to parallelize
			Game_DeleteList_DeleteAll();

			// Update loop condition
			timeSinceLastWorldStep -= GAME.physicsStep_s;
		}
		//////////////////////////// END OF PHYSICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Clear screen
		SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
		SDL_RenderClear(GAME.sdlRenderer);

		// Draw terrain
		GAME.deltaTicks_ms = SDLUtils_GetTicksAtLeast1ms(prevTerrainDrawGraphicsTicks) - prevTerrainDrawGraphicsTicks;
		GAME.deltaTime_s = GAME.deltaTicks_ms / 1000.0f;
		prevTerrainDrawGraphicsTicks += GAME.deltaTicks_ms;
        for (auto graphic_it : GAME.terrainGraphics) {
            if (graphic_it.first->draw) {
                graphic_it.first->draw(*graphic_it.first);
            }
        } // TODO Easy to parallelize

		// Pre-graphic
		GAME.deltaTicks_ms = SDLUtils_GetTicksAtLeast1ms(prevPreGraphicsTicks) - prevPreGraphicsTicks;
		GAME.deltaTime_s = GAME.deltaTicks_ms / 1000.0f;
		prevPreGraphicsTicks += GAME.deltaTicks_ms;
        for (auto monitor_it : GAME.monitors) {
            if (monitor_it.first->preGraphics) {
                monitor_it.first->preGraphics(*monitor_it.first);
            }
        } // TODO Hard to parallelize

		// Draw
		InsertionList_Sort(&GAME.drawList);
		GAME.deltaTicks_ms = SDLUtils_GetTicksAtLeast1ms(prevDrawTicks) - prevDrawTicks;
		GAME.deltaTime_s = GAME.deltaTicks_ms / 1000.0f;
		prevDrawTicks += GAME.deltaTicks_ms;
		size_t insertionListSize = InsertionList_Length(&GAME.drawList);
		for (size_t i = 0; i < insertionListSize; i++) {
			auto& gfx = GAME.graphics[InsertionList_Get(&GAME.drawList, i)];
			if (gfx.draw) {
                gfx.draw(gfx);
            }
		} // TODO Hard to parallelize

		// Draw lights
		GAME.deltaTicks_ms = SDLUtils_GetTicksAtLeast1ms(prevDrawLightsTicks) - prevDrawLightsTicks;
		GAME.deltaTime_s = GAME.deltaTicks_ms / 1000.0f;
		prevDrawLightsTicks += GAME.deltaTicks_ms;
        for (auto light_it : GAME.lights) {
            if (light_it.first->draw) {
                light_it.first->draw(*light_it.first);
            }
        } // TODO Hard to parallelize

		// Post-graphic
		GAME.deltaTicks_ms = SDLUtils_GetTicksAtLeast1ms(prevPostGraphicsTicks) - prevPostGraphicsTicks;
		GAME.deltaTime_s = GAME.deltaTicks_ms / 1000.0f;
		prevPostGraphicsTicks += GAME.deltaTicks_ms;
        for (auto monitor_it : GAME.monitors) {
            if (monitor_it.first->postGraphics) {
                monitor_it.first->postGraphics(*monitor_it.first);
            }
        } // TODO Hard to parallelize

		// Draw HUD
        GAME.leftHudUIState.update_contents();
        GAME.rightHudUIState.update_contents();
        GAME.leftHudUIState.draw();
        GAME.rightHudUIState.draw();

		// Draw envelope
		SDL_SetRenderDrawColor(GAME.sdlRenderer, 0, 0, 0, 255);
		SDL_RenderFillRect(GAME.sdlRenderer, &GAME.firstEnvelopeRect);
		SDL_RenderFillRect(GAME.sdlRenderer, &GAME.secondEnvelopeRect);

		// Present
		SDL_RenderPresent(GAME.sdlRenderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////

		unsigned end_ticks = SDL_GetTicks();
		frameTimeAccumulator += end_ticks - start_ticks;
		frameCount++;
		if (1000 < frameTimeAccumulator) {
			frameTimeAccumulator -= 1000;
			LOG_TRACE_M2V(M2_FPS, Int32, frameCount);
			frameCount = 0;
		}
	}

	SDL_DestroyRenderer(GAME.sdlRenderer);
	SDL_FreeCursor(GAME.sdlCursor);
	SDL_DestroyWindow(GAME.sdlWindow);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}
