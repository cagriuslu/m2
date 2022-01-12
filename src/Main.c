#define _CRT_SECURE_NO_WARNINGS
#include "Box2D.h"
#include "Object.h"
#include "Array.h"
#include "Markup.h"
#include "Component.h"
#include "Event.h"
#include "Pool.h"
#include "Game.h"
#include "SDLUtils.h"
#include "Log.h"
#include "Cfg.h"
#include "Pathfinder.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <string.h>

//void AudioCallback(void* userdata, uint8_t* stream, int len) {
//	for (int i = 0; i < len; i++) {
//		stream[i] = (i % 384) < 192 ? 127 : -128;
//	}
//}

int main(int argc, char **argv) {
	LOGFN_DBG();

	// Process command line arguments
	for (int i = 1; i < argc; i++) {
		const char* loglevel = "--loglevel=";
		size_t loglevelStrlen = strlen(loglevel);
		if (strncmp(argv[i], loglevel, loglevelStrlen) == 0) {
			if (strcmp(argv[i] + loglevelStrlen, "trace") == 0) {
				gCurrentLogLevel = LogLevelTrace;
				LOGXV_INF(XOK_LOG_LEVEL, Int32, LogLevelTrace);
			} else if (strcmp(argv[i] + loglevelStrlen, "debug") == 0) {
				gCurrentLogLevel = LogLevelDebug;
				LOGXV_INF(XOK_LOG_LEVEL, Int32, LogLevelDebug);
			} else if (strcmp(argv[i] + loglevelStrlen, "info") == 0) {
				gCurrentLogLevel = LogLevelInfo;
				LOGXV_INF(XOK_LOG_LEVEL, Int32, LogLevelInfo);
			} else if (strcmp(argv[i] + loglevelStrlen, "warning") == 0) {
				gCurrentLogLevel = LogLevelWarn;
				LOGXV_INF(XOK_LOG_LEVEL, Int32, LogLevelWarn);
			} else if (strcmp(argv[i] + loglevelStrlen, "error") == 0) {
				gCurrentLogLevel = LogLevelError;
				LOGXV_INF(XOK_LOG_LEVEL, Int32, LogLevelError);
			} else if (strcmp(argv[i] + loglevelStrlen, "fatal") == 0) {
				gCurrentLogLevel = LogLevelFatal;
				LOGXV_INF(XOK_LOG_LEVEL, Int32, LogLevelFatal);
			} else {
				LOG_WRN("Invalid log level");
			}
		} else {
			LOG_WRN("Invalid command line argument");
		}
	}

	GAME = calloc(1, sizeof(Game));
	GAME->tileWidth = 24;
	GAME->textureImageFilePath = "resources/24x24.png";
	GAME->textureMetaImageFilePath = "resources/24x24_META.png";
	GAME->textureMetaFilePath = "resources/24x24_META.txt";
	GAME->physicsStepPerSecond = 80.0f;
	GAME->physicsStepPeriod = 1.0f / GAME->physicsStepPerSecond;
	GAME->velocityIterations = 8;
	GAME->positionIterations = 3;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		LOGXV_FTL(XERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		LOGXV_FTL(XERR_SDL_ERROR, String, IMG_GetError());
		return -1;
	}
	if (TTF_Init() != 0) {
		LOGXV_FTL(XERR_SDL_ERROR, String, TTF_GetError());
		return -1;
	}
	Game_UpdateWindowDimensions(1600, 900);
	if ((GAME->sdlWindow = SDL_CreateWindow("cgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GAME->windowRect.w, GAME->windowRect.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == NULL) {
		LOGXV_FTL(XERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	SDL_SetWindowMinimumSize(GAME->sdlWindow, 712, 400);
	SDL_StopTextInput(); // Text input begins activated (sometimes)
	GAME->sdlCursor = SDLUtils_CreateCursor();
	SDL_SetCursor(GAME->sdlCursor);
	if ((GAME->pixelFormat = SDL_GetWindowPixelFormat(GAME->sdlWindow)) == SDL_PIXELFORMAT_UNKNOWN) {
		LOGXV_FTL(XERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	if ((GAME->sdlRenderer = SDL_CreateRenderer(GAME->sdlWindow, -1, SDL_RENDERER_ACCELERATED)) == NULL) { // SDL_RENDERER_PRESENTVSYNC
		LOGXV_FTL(XERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	SDL_Surface* textureMapSurface = IMG_Load(CFG_TEXTURE_FILE);
	if (textureMapSurface == NULL) {
		LOGXV_FTL(XERR_SDL_ERROR, String, IMG_GetError());
		return -1;
	}
	if ((GAME->sdlTexture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textureMapSurface)) == NULL) {
		LOGXV_FTL(XERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	SDL_FreeSurface(textureMapSurface);
	if ((GAME->ttfFont = TTF_OpenFont("resources/fonts/joystix/joystix-monospace.ttf", 16)) == NULL) {
		LOGXV_FTL(XERR_SDL_ERROR, String, TTF_GetError());
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

	CfgMarkupButtonType startMenuPressedButtonType;
	XErr startMenuResult = Markup_ExecuteBlocking(&CFG_MARKUP_START_MENU, &startMenuPressedButtonType);
	if (startMenuResult == XERR_QUIT) {
		return 0;
	} else if (!startMenuResult) {
		if (startMenuPressedButtonType == CFG_MARKUP_BUTTON_TYPE_NEW_GAME) {
			REFLECT_ERROR(Game_Level_Init());
			REFLECT_ERROR(Game_Level_Load(&CFG_LVL_SP000));
			REFLECT_ERROR(PathfinderMap_Init(&GAME->pathfinderMap));
			LOG_INF("Level loaded");
		} else if (startMenuPressedButtonType == CFG_MARKUP_BUTTON_TYPE_QUIT) {
			return 0;
		} else {
			// Unknown button
			return 1;
		}
	} else {
		// No specific errors are defined yet
		return 1;
	}

	float timeSinceLastWorldStep = 0.0f;
	unsigned prevPrePhysicsTicks = SDL_GetTicks();
	unsigned prevWorldStepTicks = SDL_GetTicks();
	unsigned prevPostPhysicsTicks = SDL_GetTicks();
	unsigned prevTerrainDrawGraphicsTicks = SDL_GetTicks();
	unsigned prevPreGraphicsTicks = SDL_GetTicks();
	unsigned prevDrawGraphicsTicks = SDL_GetTicks();
	unsigned prevPostGraphicsTicks = SDL_GetTicks();

	unsigned frameTimeAccumulator = 0;
	unsigned frameCount = 0;
	while (true) {
		unsigned start_ticks = SDL_GetTicks();

		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (Events_Gather(&GAME->events)) {
			// Handle quit event
			if (GAME->events.quitEvent) {
				break;
			}
			// Handle window resize event
			if (GAME->events.windowResizeEvent) {
				Game_UpdateWindowDimensions(GAME->events.windowDims.x, GAME->events.windowDims.y);
				MarkupState_UpdatePositions(&GAME->leftHudMarkupState, GAME->leftHudRect);
			}
			if (!SDL_IsTextInputActive()) {
				// Handle key events
				if (GAME->events.keysPressed[KEY_MENU]) {
					// TODO
					//goto main_menu;
				}
				if (GAME->events.keysPressed[KEY_CONSOLE]) {
					memset(GAME->consoleInput, 0, sizeof(GAME->consoleInput));
					SDL_StartTextInput();
					LOG_INF("SDL text input activated");
				}
				// Handle HUD events (mouse and key)
				CfgMarkupButtonType pressedButton;
				if (MarkupState_HandleEvents(&GAME->leftHudMarkupState, &GAME->events, &pressedButton)) {
					LOGXV_INF(XOK_BUTTON, Int32, pressedButton);
					// There are no hud buttons yet that we care about
				}
			} else {
				// Handle text input
				if (GAME->events.keysPressed[KEY_MENU]) {
					SDL_StopTextInput();
					LOG_INF("SDL text input deactivated");
				} else if (GAME->events.keysPressed[KEY_ENTER]) {
					// TODO Execute console command
					LOG_INF("Console command");
					LOG_INF(GAME->consoleInput);
					SDL_StopTextInput();
					LOG_INF("SDL text input deactivated");
				} else if (GAME->events.textInputEvent) {
					strcat(GAME->consoleInput, GAME->events.textInput);
					LOG_INF("Console buffer");
					LOG_INF(GAME->consoleInput);
				}
			}
		}
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// PHYSICS ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		uint32_t ticksSinceLastWorldStep = SDL_GetTicks() - prevWorldStepTicks;
		prevWorldStepTicks += ticksSinceLastWorldStep;
		timeSinceLastWorldStep += ticksSinceLastWorldStep / 1000.0f;
		while (GAME->physicsStepPeriod < timeSinceLastWorldStep) {
			// Pre-physics
			GAME->deltaTicks = SDL_GetTicks() - prevPrePhysicsTicks;
			prevPrePhysicsTicks += GAME->deltaTicks;
			for (ComponentMonitor* el = Pool_GetFirst(&GAME->monitors); el; el = Pool_GetNext(&GAME->monitors, el)) {
				if (el->prePhysics) {
					el->prePhysics(el);
				}
			}
			// Physics
			Box2DWorldStep(GAME->world, GAME->physicsStepPeriod, GAME->velocityIterations, GAME->positionIterations);
			for (ComponentPhysique* phy = Pool_GetFirst(&GAME->physics); phy; phy = Pool_GetNext(&GAME->physics, phy)) {
				if (phy->body) {
					Object* obj = Pool_GetById(&GAME->objects, phy->super.objId);
					if (obj) {
						obj->position = Box2DBodyGetPosition(phy->body);
					}
				}
			}
			// Post-physics
			GAME->deltaTicks = SDL_GetTicks() - prevPostPhysicsTicks;
			prevPostPhysicsTicks += GAME->deltaTicks;
			for (ComponentMonitor* el = Pool_GetFirst(&GAME->monitors); el; el = Pool_GetNext(&GAME->monitors, el)) {
				if (el->postPhysics) {
					el->postPhysics(el);
				}
			}
			Game_DeleteList_DeleteAll();
			// Update loop condition
			timeSinceLastWorldStep -= GAME->physicsStepPeriod;
		}
		//////////////////////////// END OF PHYSICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Clear screen
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
		SDL_RenderClear(GAME->sdlRenderer);
		// Draw terrain
		GAME->deltaTicks = SDL_GetTicks() - prevTerrainDrawGraphicsTicks;
		prevTerrainDrawGraphicsTicks += GAME->deltaTicks;
		for (ComponentGraphic* gfx = Pool_GetFirst(&GAME->terrainGraphics); gfx; gfx = Pool_GetNext(&GAME->terrainGraphics, gfx)) {
			if (gfx->draw) {
				gfx->draw(gfx);
			}
		}
		// Pre-graphic
		GAME->deltaTicks = SDL_GetTicks() - prevPreGraphicsTicks;
		prevPreGraphicsTicks += GAME->deltaTicks;
		for (ComponentMonitor* el = Pool_GetFirst(&GAME->monitors); el; el = Pool_GetNext(&GAME->monitors, el)) {
			if (el->preGraphics) {
				el->preGraphics(el);
			}
		}
		// Draw
		InsertionList_Sort(&GAME->drawList);
		GAME->deltaTicks = SDL_GetTicks() - prevDrawGraphicsTicks;
		prevDrawGraphicsTicks += GAME->deltaTicks;
		size_t insertionListSize = InsertionList_Length(&GAME->drawList);
		for (size_t i = 0; i < insertionListSize; i++) {
			ID graphicsId = InsertionList_Get(&GAME->drawList, i);
			ComponentGraphic* gfx = Pool_GetById(&GAME->graphics, graphicsId);
			if (gfx && gfx->draw) {
				gfx->draw(gfx);
			}
		}
		// Post-graphic
		GAME->deltaTicks = SDL_GetTicks() - prevPostGraphicsTicks;
		prevPostGraphicsTicks += GAME->deltaTicks;
		for (ComponentMonitor* el = Pool_GetFirst(&GAME->monitors); el; el = Pool_GetNext(&GAME->monitors, el)) {
			if (el->postGraphics) {
				el->postGraphics(el);
			}
		}
		// Draw Markup HUD
		MarkupState_UpdateElements(&GAME->leftHudMarkupState);
		MarkupState_Draw(&GAME->leftHudMarkupState);
		// Draw envelope
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
		SDL_RenderFillRect(GAME->sdlRenderer, &GAME->firstEnvelopeRect);
		SDL_RenderFillRect(GAME->sdlRenderer, &GAME->secondEnvelopeRect);
		// Present
		SDL_RenderPresent(GAME->sdlRenderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////

		unsigned end_ticks = SDL_GetTicks();
		frameTimeAccumulator += end_ticks - start_ticks;
		frameCount++;
		if (1000 < frameTimeAccumulator) {
			frameTimeAccumulator -= 1000;
			LOGXV_TRC(XOK_FPS, Int32, frameCount);
			frameCount = 0;
		}
	}

	SDL_DestroyRenderer(GAME->sdlRenderer);
	SDL_FreeCursor(GAME->sdlCursor);
	SDL_DestroyWindow(GAME->sdlWindow);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}
