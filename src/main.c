#include "Main.h"
#include "Box2DWrapper.h"
#include "Object.h"
#include "Array.h"
#include "Vec2I.h"
#include "EventListenerComponent.h"
#include "PhysicsComponent.h"
#include "GraphicsComponent.h"
#include "Event.h"
#include "Bucket.h"
#include "Level.h"
#include "Dialog.h"
#include "Debug.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

int gScreenWidth = 1280, gScreenHeight = 720;
float gPixelsPerMeter = 48.0;
int gTileWidth = TILE_WIDTH;
uint32_t gWindowPixelFormat;
SDL_Renderer *gRenderer;
SDL_Texture *gTextureLUT;
TTF_Font *gFont;

Level gLevel;

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	int res;

	fprintf(stderr, "Hello, world!\n");
	
	const float timeStep = 1.0f / 60.0f;
	const int velocityIterations = 8;
	const int positionIterations = 3;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	SDL_Window *window = SDL_CreateWindow("cgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gScreenWidth, gScreenHeight, SDL_WINDOW_SHOWN);
	gWindowPixelFormat = SDL_GetWindowPixelFormat(window);
	gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	gTextureLUT = SDL_CreateTextureFromSurface(gRenderer, IMG_Load("resources/" TILE_WIDTH_STR "x" TILE_WIDTH_STR ".png"));
	gFont = TTF_OpenFont("resources/fonts/joystix/joystix monospace.ttf", 16);

	bool levelLoaded = false;

main_menu:
	res = DialogMainMenu(levelLoaded);
	if (res == X_QUIT) {
		return 0;
	} else if (res == X_MAIN_MENU_RESUME) {
		// Do nothing
	} else {
		// Unload level
		if (levelLoaded) {
			LevelDeinit(&gLevel);
		}
		// Load level
		LevelInit(&gLevel);

		if (res == X_MAIN_MENU_NEW_GAME) {
			PROPAGATE_ERROR(LevelLoadTest(&gLevel));
		} else if (res == X_MAIN_MENU_LEVEL_EDITOR) {
			PROPAGATE_ERROR(LevelLoadEditor(&gLevel));
		} else {
			fprintf(stderr, "Level not found\n");
			return X_QUIT;
		}
		levelLoaded = true;
	}

	bool quit = false;
	while (!quit) {
		//unsigned start_ticks = SDL_GetTicks();

		///// EVENT HANDLING /////
		bool key = false;
		GatherEvents(&quit, NULL, &key, NULL, NULL, NULL);
		if (quit) {
			break;
		}
		if (key && IsKeyPressed(KEY_MENU)) {
			goto main_menu;
		}
		///// END OF EVENT HANDLING /////

		///// PHYSICS /////
		for (EventListenerComponent* el = BucketGetFirst(&gLevel.eventListeners); el; el = BucketGetNext(&gLevel.eventListeners, el)) {
			if (el->prePhysics) {
				el->prePhysics(el);
			}
		}
		if (gLevel.world) {
			Box2DWorldStep(gLevel.world, timeStep, velocityIterations, positionIterations);
		}
		for (PhysicsComponent* phy = BucketGetFirst(&gLevel.physics); phy; phy = BucketGetNext(&gLevel.physics, phy)) {
			if (phy->body) {
				Object* obj = BucketGetById(&gLevel.objects, phy->super.object);
				if (obj) {
					obj->position = Box2DBodyGetPosition(phy->body);
				}
			}
		}
		for (EventListenerComponent* el = BucketGetFirst(&gLevel.eventListeners); el; el = BucketGetNext(&gLevel.eventListeners, el)) {
			if (el->postPhysics) {
				el->postPhysics(el);
			}
		}
		///// END OF PHYSICS /////

		///// GRAPHICS /////
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderClear(gRenderer);
		for (GraphicsComponent* gfx = BucketGetFirst(&gLevel.terrainGraphics); gfx; gfx = BucketGetNext(&gLevel.terrainGraphics, gfx)) {
			if (gfx->draw) {
				gfx->draw(gfx);
			}
		}
		for (EventListenerComponent* el = BucketGetFirst(&gLevel.eventListeners); el; el = BucketGetNext(&gLevel.eventListeners, el)) {
			if (el->preGraphics) {
				el->preGraphics(el);
			}
		}
		InsertionListSort(&gLevel.drawList);
		size_t insertionListSize = InsertionListLength(&gLevel.drawList);
		for (size_t i = 0; i < insertionListSize; i++) {
			uint32_t* graphicsIdPtr = InsertionListGet(&gLevel.drawList, i);
			uint32_t graphicsId = *graphicsIdPtr;
			GraphicsComponent* gfx = BucketGetById(&gLevel.graphics, graphicsId);
			if (gfx && gfx->draw) {
				gfx->draw(gfx);
			}
		}
		for (EventListenerComponent* el = BucketGetFirst(&gLevel.eventListeners); el; el = BucketGetNext(&gLevel.eventListeners, el)) {
			if (el->postGraphics) {
				el->postGraphics(el);
			}
		}
		SDL_RenderPresent(gRenderer);
		///// END OF GRAPHICS /////

		//unsigned end_ticks = SDL_GetTicks();
		//fprintf(stderr, "Frame time: %u\n", end_ticks - start_ticks);
	}

	SDL_DestroyRenderer(gRenderer);
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

Level* CurrentLevel() {
	return &gLevel;
}

Vec2F CurrentPointerPositionInWorld() {
	Object* camera = BucketGetById(&CurrentLevel()->objects, CurrentLevel()->cameraId);
	Vec2F cameraPosition = camera->position;

	Vec2I pointerPosition = PointerPosition();
	Vec2I pointerPositionWRTScreenCenter = (Vec2I){ pointerPosition.x - (CurrentScreenWidth() / 2), pointerPosition.y - (CurrentScreenHeight() / 2) };
	Vec2F pointerPositionWRTCameraPos = (Vec2F){ pointerPositionWRTScreenCenter.x / CurrentPixelsPerMeter(), pointerPositionWRTScreenCenter.y / CurrentPixelsPerMeter() };
	Vec2F pointerPositionWRTWorld = Vec2FAdd(pointerPositionWRTCameraPos, cameraPosition);
	return pointerPositionWRTWorld;
}
