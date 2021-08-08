#include "Main.h"
#include "Box2D.h"
#include "Object.h"
#include "Array.h"
#include "Vec2I.h"
#include "Component.h"
#include "Event.h"
#include "Bucket.h"
#include "Level.h"
#include "Dialog.h"
#include "SDLUtils.h"
#include "Log.h"
#include "Pathfinder.h"
#include "TextureMap.h"
#include "Txt.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>

int gScreenWidth = 1600, gScreenHeight = 900;
const float gGameAreaAspectRatio = 5.0f / 4.0f;
float gPixelsPerMeter;
int gTileWidth = TILE_WIDTH;
uint32_t gWindowPixelFormat;
SDL_Renderer *gRenderer;
SDL_Texture *gTextureLUT;
TTF_Font *gFont;
TextureMap gTextureMap;

Level gLevel;
unsigned gDeltaTicks;

void SetWindowSizeAndPPM(int width, int height) {
	gScreenWidth = width;
	gScreenHeight = height;
	gPixelsPerMeter = height / 16.0f;
}

int main(int argc, char **argv) {
	LOG_TRC("main");
	(void)argc;
	(void)argv;
	int res;
	
	const float physicsStepPerSecond = 80.0f;
	const float physicsStepPeriod = 1.0f / physicsStepPerSecond;
	const int velocityIterations = 8;
	const int positionIterations = 3;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	SetWindowSizeAndPPM(gScreenWidth, gScreenHeight);
	SDL_Window *window = SDL_CreateWindow("cgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gScreenWidth, gScreenHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	SDL_SetWindowMinimumSize(window, 800, 450);
	SDL_Cursor* cursor = SDLUtils_CreateCursor();
	SDL_SetCursor(cursor);
	gWindowPixelFormat = SDL_GetWindowPixelFormat(window);
	gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED); // SDL_RENDERER_PRESENTVSYNC
	gTextureLUT = SDL_CreateTextureFromSurface(gRenderer, IMG_Load("resources/" TEXTURE_FILE_KEY ".png"));
	gFont = TTF_OpenFont("resources/fonts/joystix/joystix monospace.ttf", 16);
	TextureMap_Init(&gTextureMap, TILE_WIDTH, "resources/" TEXTURE_FILE_KEY ".png", "resources/" TEXTURE_FILE_KEY "_META.png", "resources/" TEXTURE_FILE_KEY "_META.txt");

	bool levelLoaded = false;

main_menu:
	res = DialogMainMenu(levelLoaded);
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
			PROPAGATE_ERROR(Level_LoadTest(&gLevel));
		} else if (res == X_MAIN_MENU_LEVEL_EDITOR) {
			PROPAGATE_ERROR(Level_LoadEditor(&gLevel));
		} else {
			LOG_FTL("Unknown level is selected");
			LOGTYP_FTL(LOGVAR_MENU_SELECTION, Int32, res);
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
	bool quit = false;
	while (!quit) {
		unsigned start_ticks = SDL_GetTicks();

		///// EVENT HANDLING /////
		bool windowEvent = false;
		bool key = false;
		GatherEvents(&quit, &windowEvent, &key, NULL, NULL, NULL);
		if (quit) {
			break;
		}
		if (windowEvent && IsScreenResized().x && IsScreenResized().y) {
			SetWindowSizeAndPPM(IsScreenResized().x, IsScreenResized().y);
		}
		if (key && IsKeyPressed(KEY_MENU)) {
			goto main_menu;
		}
		///// END OF EVENT HANDLING /////

		///// PHYSICS /////
		gDeltaTicks = SDL_GetTicks() - prevPrePhysicsTicks;
		prevPrePhysicsTicks += gDeltaTicks;
		for (ComponentEventListener* el = Bucket_GetFirst(&gLevel.eventListeners); el; el = Bucket_GetNext(&gLevel.eventListeners, el)) {
			if (el->prePhysics) {
				el->prePhysics(el);
			}
		}
		if (gLevel.world) {
			gDeltaTicks = SDL_GetTicks() - prevWorldStepTicks;
			timeSinceLastWorldStep += gDeltaTicks / 1000.0f;
			while (physicsStepPeriod < timeSinceLastWorldStep) {
				Box2DWorldStep(gLevel.world, physicsStepPeriod, velocityIterations, positionIterations);
				timeSinceLastWorldStep -= physicsStepPeriod;
			}
			prevWorldStepTicks += gDeltaTicks;
		}
		for (ComponentPhysics* phy = Bucket_GetFirst(&gLevel.physics); phy; phy = Bucket_GetNext(&gLevel.physics, phy)) {
			if (phy->body) {
				Object* obj = Bucket_GetById(&gLevel.objects, phy->super.objId);
				if (obj) {
					obj->position = Box2DBodyGetPosition(phy->body);
				}
			}
		}
		for (ComponentLightSource* light = Bucket_GetFirst(&gLevel.lightSources); light; light = Bucket_GetNext(&gLevel.lightSources, light)) {
			ComponentLightSource_UpdatePosition(light);
		}
		gDeltaTicks = SDL_GetTicks() - prevPostPhysicsTicks;
		prevPostPhysicsTicks += gDeltaTicks;
		for (ComponentEventListener* el = Bucket_GetFirst(&gLevel.eventListeners); el; el = Bucket_GetNext(&gLevel.eventListeners, el)) {
			if (el->postPhysics) {
				el->postPhysics(el);
			}
		}
		Level_DeleteMarkedObjects(&gLevel);
		///// END OF PHYSICS /////

		///// GRAPHICS /////
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderClear(gRenderer);
		gDeltaTicks = SDL_GetTicks() - prevTerrainDrawGraphicsTicks;
		prevTerrainDrawGraphicsTicks += gDeltaTicks;
		for (ComponentGraphics* gfx = Bucket_GetFirst(&gLevel.terrainGraphics); gfx; gfx = Bucket_GetNext(&gLevel.terrainGraphics, gfx)) {
			if (gfx->draw) {
				gfx->draw(gfx);
			}
		}
		gDeltaTicks = SDL_GetTicks() - prevPreGraphicsTicks;
		prevPreGraphicsTicks += gDeltaTicks;
		for (ComponentEventListener* el = Bucket_GetFirst(&gLevel.eventListeners); el; el = Bucket_GetNext(&gLevel.eventListeners, el)) {
			if (el->preGraphics) {
				el->preGraphics(el);
			}
		}
		InsertionList_Sort(&gLevel.drawList);
		gDeltaTicks = SDL_GetTicks() - prevDrawGraphicsTicks;
		prevDrawGraphicsTicks += gDeltaTicks;
		size_t insertionListSize = InsertionList_Length(&gLevel.drawList);
		for (size_t i = 0; i < insertionListSize; i++) {
			ID graphicsId = InsertionList_Get(&gLevel.drawList, i);
			ComponentGraphics* gfx = Bucket_GetById(&gLevel.graphics, graphicsId);
			if (gfx && gfx->draw) {
				gfx->draw(gfx);
			}
		}
		gDeltaTicks = SDL_GetTicks() - prevPostGraphicsTicks;
		prevPostGraphicsTicks += gDeltaTicks;
		for (ComponentEventListener* el = Bucket_GetFirst(&gLevel.eventListeners); el; el = Bucket_GetNext(&gLevel.eventListeners, el)) {
			if (el->postGraphics) {
				el->postGraphics(el);
			}
		}
		SDL_RenderPresent(gRenderer);
		///// END OF GRAPHICS /////

		unsigned end_ticks = SDL_GetTicks();
		frameTimeAccumulator += end_ticks - start_ticks;
		frameCount++;
		if (2000 < frameTimeAccumulator) {
			frameTimeAccumulator -= 2000;
			LOGTYP_DBG(LOGVAR_FPS, Int32, frameCount / 2);
			frameCount = 0;
		}
	}

	SDL_DestroyRenderer(gRenderer);
	SDL_FreeCursor(cursor);
	SDL_DestroyWindow(window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}

int CurrentScreenWidth() {
	return gScreenWidth;
}

int CurrentScreenHeight(){
	return gScreenHeight;
}

float CurrentPixelsPerMeter() {
	return gPixelsPerMeter;
}

int CurrentTileWidth() {
	return gTileWidth;
}

uint32_t CurrentWindowPixelFormat() {
	return gWindowPixelFormat;
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

unsigned DeltaTicks() {
	return gDeltaTicks;
}

Vec2F CurrentPointerPositionInWorld() {
	Object* camera = Bucket_GetById(&CurrentLevel()->objects, CurrentLevel()->cameraId);
	Vec2F cameraPosition = camera->position;

	Vec2I pointerPosition = PointerPosition();
	Vec2I pointerPositionWRTScreenCenter = (Vec2I){ pointerPosition.x - (CurrentScreenWidth() / 2), pointerPosition.y - (CurrentScreenHeight() / 2) };
	Vec2F pointerPositionWRTCameraPos = (Vec2F){ pointerPositionWRTScreenCenter.x / CurrentPixelsPerMeter(), pointerPositionWRTScreenCenter.y / CurrentPixelsPerMeter() };
	Vec2F pointerPositionWRTWorld = Vec2F_Add(pointerPositionWRTCameraPos, cameraPosition);
	return pointerPositionWRTWorld;
}
