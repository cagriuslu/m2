#define _CRT_SECURE_NO_WARNINGS
#include "Main.h"
#include "Box2D.h"
#include "Object.h"
#include "Array.h"
#include "Vec2I.h"
#include "Component.h"
#include "Event.h"
#include "Pool.h"
#include "Level.h"
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

SDL_Renderer *gRenderer;
SDL_Texture *gTextureLUT;
TTF_Font *gFont;
TextureMap gTextureMap;

Level gLevel;
Events gEvents;
unsigned gDeltaTicks;
char gConsoleInput[1024];

int main(int argc, char **argv) {
	LOG_TRC("main");
	(void)argc;
	(void)argv;
	XErr res;

	Game *game = calloc(1, sizeof(Game));
	game->tileWidth = 24;
	game->textureImageFilePath = "resources/24x24.png";
	game->textureMetaImageFilePath = "resources/24x24_META.png";
	game->textureMetaFilePath = "resources/24x24_META.txt";
	game->window.windowWidth = 1600;
	game->window.windowHeight = 900;
	game->physicsStepPerSecond = 80.0f;
	game->physicsStepPeriod = 1.0f / game->physicsStepPerSecond;
	game->velocityIterations = 8;
	game->positionIterations = 3;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Window_SetWidthHeight(&game->window, game->window.windowWidth, game->window.windowHeight);
	game->window.sdlWindow = SDL_CreateWindow("cgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, game->window.windowWidth, game->window.windowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(game->window.sdlWindow, 712, 400);
	SDL_StopTextInput(); // Text input begins activated (sometimes)
	game->window.cursor = SDLUtils_CreateCursor();
	SDL_SetCursor(game->window.cursor);
	game->window.pixelFormat = SDL_GetWindowPixelFormat(game->window.sdlWindow);
	gRenderer = SDL_CreateRenderer(game->window.sdlWindow, -1, SDL_RENDERER_ACCELERATED); // SDL_RENDERER_PRESENTVSYNC
	gTextureLUT = SDL_CreateTextureFromSurface(gRenderer, IMG_Load("resources/24x24.png"));
	gFont = TTF_OpenFont("resources/fonts/joystix/joystix monospace.ttf", 16);
	TextureMap_Init(&gTextureMap, game->tileWidth, game->textureImageFilePath, game->textureMetaImageFilePath, game->textureMetaFilePath);

	bool levelLoaded = false;

main_menu:
	res = DialogMainMenu(game, levelLoaded);
	if (res == XERR_QUIT) {
		return 0;
	} else if (res == X_MAIN_MENU_RESUME) {
		// Do nothing
	} else {
		// Unload level
		if (levelLoaded) {
			Level_Term(&gLevel);
		}
		// Load level
		Level_Init(&gLevel);

		if (res == X_MAIN_MENU_NEW_GAME) {
			PROPAGATE_ERROR(Level_LoadTest(&gLevel, game));
		} else if (res == X_MAIN_MENU_LEVEL_EDITOR) {
			PROPAGATE_ERROR(Level_LoadEditor(&gLevel, game));
		} else {
			LOG_FTL("Unknown level is selected");
			LOGOBJ_FTL(LOGVAR_MENU_SELECTION, Int32, res);
			return XERR_QUIT;
		}
		levelLoaded = true;
	}
	PathfinderMap_InitFromLevel(&gLevel.pathfinderMap, &gLevel);
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
		if (Events_Gather(&gEvents)) {
			if (gEvents.quitEvent) {
				break;
			}
			if (gEvents.windowResizeEvent) {
				Window_SetWidthHeight(&game->window, gEvents.windowDims.x, gEvents.windowDims.y);
			}
			if (!SDL_IsTextInputActive()) {
				if (gEvents.keysPressed[KEY_MENU]) {
					goto main_menu;
				}
				if (gEvents.keysPressed[KEY_CONSOLE]) {
					memset(gConsoleInput, 0, sizeof(gConsoleInput));
					SDL_StartTextInput();
					LOG_INF("SDL text input activated");
				}
			} else {
				if (gEvents.keysPressed[KEY_MENU]) {
					SDL_StopTextInput();
					LOG_INF("SDL text input deactivated");
				} else if (gEvents.keysPressed[KEY_ENTER]) {
					// TODO Execute console command
					LOGOBJ_INF("Console command", String, gConsoleInput);
					SDL_StopTextInput();
					LOG_INF("SDL text input deactivated");
				} else if (gEvents.textInputEvent) {
					strcat(gConsoleInput, gEvents.textInput);
					LOGOBJ_INF("Console buffer", String, gConsoleInput);
				}
			}
		}
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// PHYSICS ////////////////////////////////
		////////////////////////////////////////////////////////////////////////
		gDeltaTicks = SDL_GetTicks() - prevPrePhysicsTicks;
		prevPrePhysicsTicks += gDeltaTicks;
		for (ComponentEventListener* el = Pool_GetFirst(&gLevel.eventListeners); el; el = Pool_GetNext(&gLevel.eventListeners, el)) {
			if (el->prePhysics) {
				el->prePhysics(el, game);
			}
		}
		if (gLevel.world) {
			gDeltaTicks = SDL_GetTicks() - prevWorldStepTicks;
			timeSinceLastWorldStep += gDeltaTicks / 1000.0f;
			while (game->physicsStepPeriod < timeSinceLastWorldStep) {
				Box2DWorldStep(gLevel.world, game->physicsStepPeriod, game->velocityIterations, game->positionIterations);
				timeSinceLastWorldStep -= game->physicsStepPeriod;
			}
			prevWorldStepTicks += gDeltaTicks;
		}
		for (ComponentPhysics* phy = Pool_GetFirst(&gLevel.physics); phy; phy = Pool_GetNext(&gLevel.physics, phy)) {
			if (phy->body) {
				Object* obj = Pool_GetById(&gLevel.objects, phy->super.objId);
				if (obj) {
					obj->position = Box2DBodyGetPosition(phy->body);
				}
			}
		}
		for (ComponentLightSource* light = Pool_GetFirst(&gLevel.lightSources); light; light = Pool_GetNext(&gLevel.lightSources, light)) {
			ComponentLightSource_UpdatePosition(light);
		}
		gDeltaTicks = SDL_GetTicks() - prevPostPhysicsTicks;
		prevPostPhysicsTicks += gDeltaTicks;
		for (ComponentEventListener* el = Pool_GetFirst(&gLevel.eventListeners); el; el = Pool_GetNext(&gLevel.eventListeners, el)) {
			if (el->postPhysics) {
				el->postPhysics(el);
			}
		}
		Level_DeleteMarkedObjects(&gLevel);
		//////////////////////////// END OF PHYSICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderClear(gRenderer);
		// Draw terrain
		gDeltaTicks = SDL_GetTicks() - prevTerrainDrawGraphicsTicks;
		prevTerrainDrawGraphicsTicks += gDeltaTicks;
		for (ComponentGraphics* gfx = Pool_GetFirst(&gLevel.terrainGraphics); gfx; gfx = Pool_GetNext(&gLevel.terrainGraphics, gfx)) {
			if (gfx->draw) {
				gfx->draw(gfx, game);
			}
		}
		// Pre-graphics
		gDeltaTicks = SDL_GetTicks() - prevPreGraphicsTicks;
		prevPreGraphicsTicks += gDeltaTicks;
		for (ComponentEventListener* el = Pool_GetFirst(&gLevel.eventListeners); el; el = Pool_GetNext(&gLevel.eventListeners, el)) {
			if (el->preGraphics) {
				el->preGraphics(el);
			}
		}
		// Draw
		InsertionList_Sort(&gLevel.drawList);
		gDeltaTicks = SDL_GetTicks() - prevDrawGraphicsTicks;
		prevDrawGraphicsTicks += gDeltaTicks;
		size_t insertionListSize = InsertionList_Length(&gLevel.drawList);
		for (size_t i = 0; i < insertionListSize; i++) {
			ID graphicsId = InsertionList_Get(&gLevel.drawList, i);
			ComponentGraphics* gfx = Pool_GetById(&gLevel.graphics, graphicsId);
			if (gfx && gfx->draw) {
				gfx->draw(gfx, game);
			}
		}
		// Draw HUD background
		SDL_SetRenderDrawColor(gRenderer, 5, 5, 5, 255);
		SDL_RenderFillRect(gRenderer, &game->window.leftHudRect);
		SDL_RenderFillRect(gRenderer, &game->window.rightHudRect);
		// Draw HUD
		Hud_Draw(&gLevel.hud, game);
		// Post-graphics
		gDeltaTicks = SDL_GetTicks() - prevPostGraphicsTicks;
		prevPostGraphicsTicks += gDeltaTicks;
		for (ComponentEventListener* el = Pool_GetFirst(&gLevel.eventListeners); el; el = Pool_GetNext(&gLevel.eventListeners, el)) {
			if (el->postGraphics) {
				el->postGraphics(el);
			}
		}
		// Draw envelope
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderFillRect(gRenderer, &game->window.firstEnvelopeRect);
		SDL_RenderFillRect(gRenderer, &game->window.secondEnvelopeRect);
		// Present
		SDL_RenderPresent(gRenderer);
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

	SDL_DestroyRenderer(gRenderer);
	SDL_FreeCursor(game->window.cursor);
	SDL_DestroyWindow(game->window.sdlWindow);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}

SDL_Renderer* CurrentRenderer() {
	return gRenderer;
}

SDL_Texture* CurrentTextureLUT() {
	return gTextureLUT;
}

TTF_Font* CurrentFont() {
	return gFont;
}

TextureMap* CurrentTextureMap() {
	return &gTextureMap;
}

Level* CurrentLevel() {
	return &gLevel;
}

Events* CurrentEvents() {
	return &gEvents;
}

unsigned DeltaTicks() {
	return gDeltaTicks;
}

char* ConsoleInputBuffer() {
	return gConsoleInput;
}

Vec2F CurrentPointerPositionInWorld(Game *game) {
	Object* camera = Pool_GetById(&CurrentLevel()->objects, CurrentLevel()->cameraId);
	Vec2F cameraPosition = camera->position;

	Vec2I pointerPosition = gEvents.mousePosition;
	Vec2I pointerPositionWRTScreenCenter = (Vec2I){ pointerPosition.x - (game->window.windowWidth / 2), pointerPosition.y - (game->window.windowHeight / 2) };
	Vec2F pointerPositionWRTCameraPos = (Vec2F){ pointerPositionWRTScreenCenter.x / game->window.pixelsPerMeter, pointerPositionWRTScreenCenter.y / game->window.pixelsPerMeter };
	Vec2F pointerPositionWRTWorld = Vec2F_Add(pointerPositionWRTCameraPos, cameraPosition);
	return pointerPositionWRTWorld;
}
