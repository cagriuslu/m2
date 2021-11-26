#define _CRT_SECURE_NO_WARNINGS
#include "Main.h"
#include "Box2D.h"
#include "Object.h"
#include "Array.h"
#include "Vec2I.h"
#include "Component.h"
#include "Event.h"
#include "Pool.h"
#include "Dialog.h"
#include "Game.h"
#include "SDLUtils.h"
#include "Log.h"
#include "Pathfinder.h"
#include "TextureMap.h"
#include "Txt.h"
#include "String.h"
#include "VSON.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

SDL_Texture *gTextureLUT;
TextureMap gTextureMap;

int main(int argc, char **argv) {
	LOG_TRC("main");
	(void)argc;
	(void)argv;
	XErr res;

	GAME = calloc(1, sizeof(Game));
	GAME->tileWidth = 24;
	GAME->textureImageFilePath = "resources/24x24.png";
	GAME->textureMetaImageFilePath = "resources/24x24_META.png";
	GAME->textureMetaFilePath = "resources/24x24_META.txt";
	GAME->windowWidth = 1600;
	GAME->windowHeight = 900;
	GAME->physicsStepPerSecond = 80.0f;
	GAME->physicsStepPeriod = 1.0f / GAME->physicsStepPerSecond;
	GAME->velocityIterations = 8;
	GAME->positionIterations = 3;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Game_SetWidthHeight(GAME, GAME->windowWidth, GAME->windowHeight);
	GAME->sdlWindow = SDL_CreateWindow("cgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, GAME->windowWidth, GAME->windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(GAME->sdlWindow, 712, 400);
	SDL_StopTextInput(); // Text input begins activated (sometimes)
	GAME->sdlCursor = SDLUtils_CreateCursor();
	SDL_SetCursor(GAME->sdlCursor);
	GAME->pixelFormat = SDL_GetWindowPixelFormat(GAME->sdlWindow);
	GAME->sdlRenderer = SDL_CreateRenderer(GAME->sdlWindow, -1, SDL_RENDERER_ACCELERATED); // SDL_RENDERER_PRESENTVSYNC
	gTextureLUT = SDL_CreateTextureFromSurface(GAME->sdlRenderer, IMG_Load("resources/24x24.png"));
	GAME->ttfFont = TTF_OpenFont("resources/fonts/joystix/joystix monospace.ttf", 16);
	TextureMap_Init(&gTextureMap, GAME->tileWidth, GAME->textureImageFilePath, GAME->textureMetaImageFilePath, GAME->textureMetaFilePath);

	main_menu:
	res = DialogMainMenu(GAME->levelLoaded);
	if (res == XERR_QUIT) {
		return 0;
	} else if (res == X_MAIN_MENU_RESUME) {
		// Do nothing
	} else {
		Game_Level_Init(GAME);
		if (res == X_MAIN_MENU_NEW_GAME) {
			PROPAGATE_ERROR(Game_Level_LoadTest(GAME));
		} else if (res == X_MAIN_MENU_LEVEL_EDITOR) {
			PROPAGATE_ERROR(Game_Level_LoadEditor(GAME));
		} else {
			LOG_FTL("Unknown level is selected");
			LOGOBJ_FTL(LOGVAR_MENU_SELECTION, Int32, res);
			return XERR_QUIT;
		}
	}
	PathfinderMap_Init(&GAME->pathfinderMap);
	LOG_INF("Level loaded");

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
			if (GAME->events.quitEvent) {
				break;
			}
			if (GAME->events.windowResizeEvent) {
				Game_SetWidthHeight(GAME, GAME->events.windowDims.x, GAME->events.windowDims.y);
			}
			if (!SDL_IsTextInputActive()) {
				if (GAME->events.keysPressed[KEY_MENU]) {
					goto main_menu;
				}
				if (GAME->events.keysPressed[KEY_CONSOLE]) {
					memset(GAME->consoleInput, 0, sizeof(GAME->consoleInput));
					SDL_StartTextInput();
					LOG_INF("SDL text input activated");
				}
			} else {
				if (GAME->events.keysPressed[KEY_MENU]) {
					SDL_StopTextInput();
					LOG_INF("SDL text input deactivated");
				} else if (GAME->events.keysPressed[KEY_ENTER]) {
					// TODO Execute console command
					LOGOBJ_INF("Console command", String, GAME->consoleInput);
					SDL_StopTextInput();
					LOG_INF("SDL text input deactivated");
				} else if (GAME->events.textInputEvent) {
					strcat(GAME->consoleInput, GAME->events.textInput);
					LOGOBJ_INF("Console buffer", String, GAME->consoleInput);
				}
			}
		}
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// PHYSICS ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		GAME->deltaTicks = SDL_GetTicks() - prevPrePhysicsTicks;
		prevPrePhysicsTicks += GAME->deltaTicks;
		for (ComponentEventListener* el = Pool_GetFirst(&GAME->eventListeners); el; el = Pool_GetNext(&GAME->eventListeners, el)) {
			if (el->prePhysics) {
				el->prePhysics(el);
			}
		}
		if (GAME->world) {
			GAME->deltaTicks = SDL_GetTicks() - prevWorldStepTicks;
			timeSinceLastWorldStep += GAME->deltaTicks / 1000.0f;
			while (GAME->physicsStepPeriod < timeSinceLastWorldStep) {
				Box2DWorldStep(GAME->world, GAME->physicsStepPeriod, GAME->velocityIterations, GAME->positionIterations);
				timeSinceLastWorldStep -= GAME->physicsStepPeriod;
			}
			prevWorldStepTicks += GAME->deltaTicks;
		}
		for (ComponentPhysics* phy = Pool_GetFirst(&GAME->physics); phy; phy = Pool_GetNext(&GAME->physics, phy)) {
			if (phy->body) {
				Object* obj = Pool_GetById(&GAME->objects, phy->super.objId);
				if (obj) {
					obj->position = Box2DBodyGetPosition(phy->body);
				}
			}
		}
		GAME->deltaTicks = SDL_GetTicks() - prevPostPhysicsTicks;
		prevPostPhysicsTicks += GAME->deltaTicks;
		for (ComponentEventListener* el = Pool_GetFirst(&GAME->eventListeners); el; el = Pool_GetNext(&GAME->eventListeners, el)) {
			if (el->postPhysics) {
				el->postPhysics(el);
			}
		}
		Game_Level_DeleteMarkedObjects(GAME);
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
		for (ComponentGraphics* gfx = Pool_GetFirst(&GAME->terrainGraphics); gfx; gfx = Pool_GetNext(&GAME->terrainGraphics, gfx)) {
			if (gfx->draw) {
				gfx->draw(gfx);
			}
		}
		// Pre-graphics
		GAME->deltaTicks = SDL_GetTicks() - prevPreGraphicsTicks;
		prevPreGraphicsTicks += GAME->deltaTicks;
		for (ComponentEventListener* el = Pool_GetFirst(&GAME->eventListeners); el; el = Pool_GetNext(&GAME->eventListeners, el)) {
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
			ComponentGraphics* gfx = Pool_GetById(&GAME->graphics, graphicsId);
			if (gfx && gfx->draw) {
				gfx->draw(gfx);
			}
		}
		// Draw HUD background
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 5, 5, 5, 255);
		SDL_RenderFillRect(GAME->sdlRenderer, &GAME->leftHudRect);
		SDL_RenderFillRect(GAME->sdlRenderer, &GAME->rightHudRect);
		// Draw HUD
		Hud_Draw(&GAME->hud);
		// Post-graphics
		GAME->deltaTicks = SDL_GetTicks() - prevPostGraphicsTicks;
		prevPostGraphicsTicks += GAME->deltaTicks;
		for (ComponentEventListener* el = Pool_GetFirst(&GAME->eventListeners); el; el = Pool_GetNext(&GAME->eventListeners, el)) {
			if (el->postGraphics) {
				el->postGraphics(el);
			}
		}
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
		if (2000 < frameTimeAccumulator) {
			frameTimeAccumulator -= 2000;
			LOGOBJ_DBG(LOGVAR_FPS, Int32, frameCount / 2);
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

SDL_Texture* CurrentTextureLUT() {
	return gTextureLUT;
}

TextureMap* CurrentTextureMap() {
	return &gTextureMap;
}
