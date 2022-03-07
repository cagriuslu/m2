#define _CRT_SECURE_NO_WARNINGS
#include "Box2D.h"
#include "Object.h"
#include "Array.h"
#include "UI.h"
#include "Component.h"
#include "Event.h"
#include "Pool.h"
#include "Game.h"
#include "SDLUtils.h"
#include "Def.h"
#include "Cfg.h"
#include "Pathfinder.h"
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

	GAME = calloc(1, sizeof(Game));
	GAME->tileWidth = CFG_TILE_SIZE;
	GAME->physicsStepPerSecond = 80.0f;
	GAME->physicsStepPeriod = 1.0f / GAME->physicsStepPerSecond;
	GAME->velocityIterations = 8;
	GAME->positionIterations = 3;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, IMG_GetError());
		return -1;
	}
	if (TTF_Init() != 0) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, TTF_GetError());
		return -1;
	}
	Game_UpdateWindowDimensions(1600, 900);
	if ((GAME->sdlWindow = SDL_CreateWindow("m2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GAME->windowRect.w, GAME->windowRect.h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)) == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	SDL_SetWindowMinimumSize(GAME->sdlWindow, 712, 400);
	SDL_StopTextInput(); // Text input begins activated (sometimes)
	GAME->sdlCursor = SDLUtils_CreateCursor();
	SDL_SetCursor(GAME->sdlCursor);
	if ((GAME->pixelFormat = SDL_GetWindowPixelFormat(GAME->sdlWindow)) == SDL_PIXELFORMAT_UNKNOWN) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	if ((GAME->sdlRenderer = SDL_CreateRenderer(GAME->sdlWindow, -1, SDL_RENDERER_ACCELERATED)) == NULL) { // SDL_RENDERER_PRESENTVSYNC
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	SDL_Surface* textureMapSurface = IMG_Load(CFG_TEXTURE_FILE);
	if (textureMapSurface == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, IMG_GetError());
		return -1;
	}
	if ((GAME->sdlTexture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textureMapSurface)) == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	SDL_SetTextureColorMod(GAME->sdlTexture, 127, 127, 127);
	SDL_FreeSurface(textureMapSurface);
	SDL_Surface* textureMaskSurface = IMG_Load(CFG_TEXTURE_MASK_FILE);
	if (textureMaskSurface == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, IMG_GetError());
		return -1;
	}
	if ((GAME->sdlTextureMask = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textureMaskSurface)) == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	SDL_FreeSurface(textureMaskSurface);
	SDL_Surface* lightSurface = IMG_Load("resources/RadialGradient-WhiteBlack.png");
	if (lightSurface == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, IMG_GetError());
		return -1;
	}
	if ((GAME->sdlLightTexture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, lightSurface)) == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, SDL_GetError());
		return -1;
	}
	SDL_FreeSurface(lightSurface);
	SDL_SetTextureBlendMode(GAME->sdlLightTexture, SDL_BLENDMODE_MUL);
	SDL_SetTextureAlphaMod(GAME->sdlLightTexture, 0);
	SDL_SetTextureColorMod(GAME->sdlLightTexture, 127, 127, 127);
	if ((GAME->ttfFont = TTF_OpenFont("resources/fonts/joystix/joystix-monospace.ttf", 16)) == NULL) {
		LOG_FATAL_M2V(M2ERR_SDL_ERROR, String, TTF_GetError());
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

	M2Err result = GameEntryPoint_InitLauncher(&GAME->gameEntryPoint);
	if (result) {
		LOG_FATAL_M2(result);
		return 1;
	}
	CfgUIButtonType launcherButton;
	result = UI_ExecuteBlocking(GAME->gameEntryPoint.entryUi, &launcherButton);
	if (result == M2ERR_QUIT) {
		return 0;
	} else if (result) {
		LOG_FATAL_M2(result);
		return 1;
	}
	M2Err handlerResult = GAME->gameEntryPoint.entryUiButtonHandler(launcherButton);
	if (handlerResult) {
		LOG_ERROR_M2(handlerResult);
	}

	CfgUIButtonType startMenuPressedButtonType;
	M2Err startMenuResult = UI_ExecuteBlocking(&CFG_UI_STARTMENU, &startMenuPressedButtonType);
	if (startMenuResult == M2ERR_QUIT) {
		return 0;
	} else if (!startMenuResult) {
		if (startMenuPressedButtonType == CFG_UI_BUTTON_TYPE_NEW_GAME) {
			M2ERR_REFLECT(Game_Level_Init());
			M2ERR_REFLECT(Game_Level_Load(&CFG_LVL_SP000));
			M2ERR_REFLECT(PathfinderMap_Init(&GAME->pathfinderMap));
			LOG_INFO("Level loaded");
		} else if (startMenuPressedButtonType == CFG_UI_BUTTON_TYPE_QUIT) {
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
		if (Events_Gather(&GAME->events)) {
			// Handle quit event
			if (GAME->events.quitEvent) { break; }
			// Handle window resize event
			if (GAME->events.windowResizeEvent) {
				Game_UpdateWindowDimensions(GAME->events.windowDims.x, GAME->events.windowDims.y);
				UIState_UpdatePositions(&GAME->leftHudUIState, GAME->leftHudRect);
				UIState_UpdatePositions(&GAME->rightHudUIState, GAME->rightHudRect);
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
					LOG_INFO("SDL text input activated");
				}
				// Handle HUD events (mouse and key)
				CfgUIButtonType pressedButton;
				if (UIState_HandleEvents(&GAME->leftHudUIState, &GAME->events, &pressedButton)) {
					LOG_INFO_M2V(M2_BUTTON, Int32, pressedButton);
					// There are no hud buttons yet that we care about
				}
				if (UIState_HandleEvents(&GAME->rightHudUIState, &GAME->events, &pressedButton)) {
					LOG_INFO_M2V(M2_BUTTON, Int32, pressedButton);
					// There are no hud buttons yet that we care about
				}
			} else {
				// Handle text input
				if (GAME->events.keysPressed[KEY_MENU]) {
					SDL_StopTextInput();
					LOG_INFO("SDL text input deactivated");
				} else if (GAME->events.keysPressed[KEY_ENTER]) {
					// TODO Execute console command
					LOG_INFO("Console command");
					LOG_INFO(GAME->consoleInput);
					SDL_StopTextInput();
					LOG_INFO("SDL text input deactivated");
				} else if (GAME->events.textInputEvent) {
					strcat(GAME->consoleInput, GAME->events.textInput);
					LOG_INFO("Console buffer");
					LOG_INFO(GAME->consoleInput);
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
		while (GAME->physicsStepPeriod < timeSinceLastWorldStep) {
			// Pre-physics
			GAME->deltaTicks = SDLUtils_GetTicksAtLeast1ms(prevPrePhysicsTicks) - prevPrePhysicsTicks;
			GAME->deltaTime = GAME->deltaTicks / 1000.0f;
			prevPrePhysicsTicks += GAME->deltaTicks;
			for (ComponentMonitor* el = Pool_GetFirst(&GAME->monitors); el; el = Pool_GetNext(&GAME->monitors, el)) {
				if (el->prePhysics) { el->prePhysics(el); }
			} // TODO Hard to parallelize
			Game_DeleteList_DeleteAll();
			// Physics
			Box2DWorldStep(GAME->world, GAME->physicsStepPeriod, GAME->velocityIterations, GAME->positionIterations);
			for (ComponentPhysique* phy = Pool_GetFirst(&GAME->physics); phy && phy->body; phy = Pool_GetNext(&GAME->physics, phy)) {
				Object* obj = Pool_GetById(&GAME->objects, phy->super.objId); M2ASSERT(obj);
				obj->position = Box2DBodyGetPosition(phy->body);
			} // TODO Easy to parallelize
			Game_DeleteList_DeleteAll();
			// Post-physics
			GAME->deltaTicks = SDLUtils_GetTicksAtLeast1ms(prevPostPhysicsTicks) - prevPostPhysicsTicks;
			GAME->deltaTime = GAME->deltaTicks / 1000.0f;
			prevPostPhysicsTicks += GAME->deltaTicks;
			for (ComponentMonitor* el = Pool_GetFirst(&GAME->monitors); el; el = Pool_GetNext(&GAME->monitors, el)) {
				if (el->postPhysics) { el->postPhysics(el); }
			} // TODO Hard to parallelize
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
		GAME->deltaTicks = SDLUtils_GetTicksAtLeast1ms(prevTerrainDrawGraphicsTicks) - prevTerrainDrawGraphicsTicks;
		GAME->deltaTime = GAME->deltaTicks / 1000.0f;
		prevTerrainDrawGraphicsTicks += GAME->deltaTicks;
		for (ComponentGraphic* gfx = Pool_GetFirst(&GAME->terrainGraphics); gfx; gfx = Pool_GetNext(&GAME->terrainGraphics, gfx)) {
			if (gfx->draw) { gfx->draw(gfx); }
		} // TODO Easy to parallelize
		// Pre-graphic
		GAME->deltaTicks = SDLUtils_GetTicksAtLeast1ms(prevPreGraphicsTicks) - prevPreGraphicsTicks;
		GAME->deltaTime = GAME->deltaTicks / 1000.0f;
		prevPreGraphicsTicks += GAME->deltaTicks;
		for (ComponentMonitor* el = Pool_GetFirst(&GAME->monitors); el; el = Pool_GetNext(&GAME->monitors, el)) {
			if (el->preGraphics) { el->preGraphics(el); }
		} // TODO Hard to parallelize
		// Draw
		InsertionList_Sort(&GAME->drawList);
		GAME->deltaTicks = SDLUtils_GetTicksAtLeast1ms(prevDrawTicks) - prevDrawTicks;
		GAME->deltaTime = GAME->deltaTicks / 1000.0f;
		prevDrawTicks += GAME->deltaTicks;
		size_t insertionListSize = InsertionList_Length(&GAME->drawList);
		for (size_t i = 0; i < insertionListSize; i++) {
			ComponentGraphic* gfx = Pool_GetById(&GAME->graphics, InsertionList_Get(&GAME->drawList, i));
			if (gfx && gfx->draw) { gfx->draw(gfx); }
		} // TODO Hard to parallelize
		// Draw lights
		GAME->deltaTicks = SDLUtils_GetTicksAtLeast1ms(prevDrawLightsTicks) - prevDrawLightsTicks;
		GAME->deltaTime = GAME->deltaTicks / 1000.0f;
		prevDrawLightsTicks += GAME->deltaTicks;
		for (ComponentLight* lig = Pool_GetFirst(&GAME->lights); lig; lig = Pool_GetNext(&GAME->lights, lig)) {
			if (lig->draw) { lig->draw(lig); }
		} // TODO Hard to parallelize
		// Post-graphic
		GAME->deltaTicks = SDLUtils_GetTicksAtLeast1ms(prevPostGraphicsTicks) - prevPostGraphicsTicks;
		GAME->deltaTime = GAME->deltaTicks / 1000.0f;
		prevPostGraphicsTicks += GAME->deltaTicks;
		for (ComponentMonitor* el = Pool_GetFirst(&GAME->monitors); el; el = Pool_GetNext(&GAME->monitors, el)) {
			if (el->postGraphics) { el->postGraphics(el); }
		} // TODO Hard to parallelize
		// Draw HUD
		UIState_UpdateElements(&GAME->leftHudUIState);
		UIState_UpdateElements(&GAME->rightHudUIState);
		UIState_Draw(&GAME->leftHudUIState);
		UIState_Draw(&GAME->rightHudUIState);
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
			LOG_TRACE_M2V(M2_FPS, Int32, frameCount);
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
