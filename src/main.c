#include "Main.h"
#include "Blueprint.h"
#include "Box2DWrapper.h"
#include "Array.h"
#include "Player.h"
#include "Camera.h"
#include "Vec2I.h"
#include "DrawList.h"
#include "Event.h"
#include "Terrain.h"
#include "Bucket.h"
#include "Level.h"
#include "Dialog.h"
#include "Debug.h"
#include "ObjectStore.h"
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

Box2DWorld *gWorld;
Box2DContactListener* gContactListener;
ObjectStore gObjectStore;
DrawList gDrawList;

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
	ObjectStoreInit(&gObjectStore);

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
			LevelUnload();
			DrawListDeinit(&gDrawList);
			ObjectStoreDestroyAllObjects(&gObjectStore);
			Box2DWorldDestroy(gWorld);
			Box2DContactListenerDestroy(gContactListener);
		}
		// Load level
		gWorld = Box2DWorldCreate((Vec2F) {0.0, 0.0});
		gContactListener = Box2DContactListenerRegister(ObjectContactCB);
		Box2DWorldSetContactListener(gWorld, gContactListener);
		DrawListInit(&gDrawList);

		if (res == X_MAIN_MENU_NEW_GAME) {
			PROPAGATE_ERROR(LevelTestLoad());
		} else if (res == X_MAIN_MENU_LEVEL_EDITOR) {
			PROPAGATE_ERROR(LevelEditorLoad());
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
		for (GameObject* obj = ObjectStoreGetFirstObject(&gObjectStore); obj; obj = ObjectStoreGetNextObject(&gObjectStore, obj)) {
			if (obj->prePhysics) {
				obj->prePhysics(obj);
			}
		}
		Box2DWorldStep(gWorld, timeStep, velocityIterations, positionIterations);
		for (GameObject* obj = ObjectStoreGetFirstObject(&gObjectStore); obj; obj = ObjectStoreGetNextObject(&gObjectStore, obj)) {
			if (obj->body) {
				obj->pos = Box2DBodyGetPosition(obj->body);
			}
		}
		for (GameObject* obj = ObjectStoreGetFirstObject(&gObjectStore); obj; obj = ObjectStoreGetNextObject(&gObjectStore, obj)) {
			if (obj->postPhysics) {
				obj->postPhysics(obj);
			}
		}
		///// END OF PHYSICS /////

		///// GRAPHICS /////
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderClear(gRenderer);
		// Draw terrain first
		GameObject* terrain = ObjectStoreGetObjectByIndex(&gObjectStore, TERRAIN_INDEX);
		if (terrain->ovrdGraphics) {
			terrain->ovrdGraphics(terrain);
		}
		DrawListSort(&gDrawList);
		size_t drawListSize = DrawListLength(&gDrawList);
		for (size_t i = 0; i < drawListSize; i++) {
			GameObject *obj = DrawListGet(&gDrawList, i);
			if (obj->preGraphics) {
				obj->preGraphics(obj);
			}
			if (obj->ovrdGraphics) {
				obj->ovrdGraphics(obj);
			}
			if (obj->postGraphics) {
				obj->postGraphics(obj);
			}
		}
		SDL_RenderPresent(gRenderer);
		///// END OF GRAPHICS /////

		// TODO remove objects marked for deletion from array and drawlist

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

Box2DWorld* CurrentWorld() {
	return gWorld;
}

ObjectStore* CurrentObjectStore() {
	return &gObjectStore;
}

DrawList* CurrentDrawList() {
	return &gDrawList;
}

Vec2F CurrentPointerPositionInWorld() {
	GameObject* camera = ObjectStoreGetObjectByIndex(CurrentObjectStore(), CAMERA_INDEX);
	Vec2F cameraPosition = camera->pos;

	Vec2I pointerPosition = PointerPosition();
	Vec2I pointerPositionWRTScreenCenter = (Vec2I){ pointerPosition.x - (CurrentScreenWidth() / 2), pointerPosition.y - (CurrentScreenHeight() / 2) };
	Vec2F pointerPositionWRTCameraPos = (Vec2F){ pointerPositionWRTScreenCenter.x / CurrentPixelsPerMeter(), pointerPositionWRTScreenCenter.y / CurrentPixelsPerMeter() };
	Vec2F pointerPositionWRTWorld = Vec2FAdd(pointerPositionWRTCameraPos, cameraPosition);
	return pointerPositionWRTWorld;
}
