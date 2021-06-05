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

Box2DWorld *gWorld;
Box2DContactListener* gContactListener;
Array gObjects;
DrawList gDrawList;

int main() {
	int res;

	fprintf(stderr, "Hello, world!\n");
	
	const int SCREEN_HALF_WIDTH = gScreenWidth / 2;
	const int SCREEN_HALF_HEIGHT = gScreenHeight / 2;
	const float timeStep = 1.0 / 60.0;
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
			LevelUnload();
			DrawListDeinit(&gDrawList);
			ArrayDeinit(&gObjects);
			Box2DWorldDestroy(gWorld);
			Box2DContactListenerDestroy(gContactListener);
		}
		// Load level
		gWorld = Box2DWorldCreate((Vec2F) {0.0, 0.0});
		gContactListener = Box2DContactListenerRegister(ObjectContactCB);
		Box2DWorldSetContactListener(gWorld, gContactListener);
		ArrayInit(&gObjects, sizeof(Object));
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

	Object *camera = ArrayGet(&gObjects, CAMERA_INDEX);

	bool quit = false;
	while (!quit) {
		unsigned start_ticks = SDL_GetTicks();

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
		for (size_t i = 0; i < ArrayLength(&gObjects); i++) {
			Object *obj = ArrayGet(&gObjects, i);
			if (obj->prePhysics) {
				obj->prePhysics(obj);
			}
		}
		Box2DWorldStep(gWorld, timeStep, velocityIterations, positionIterations);
		for (size_t i = 0; i < ArrayLength(&gObjects); i++) {
			Object *obj = ArrayGet(&gObjects, i);
			if (obj->body) {
				obj->pos = Box2DBodyGetPosition(obj->body);
				obj->angle = Box2DBodyGetAngle(obj->body);
			}
		}
		for (size_t i = 0; i < ArrayLength(&gObjects); i++) {
			Object *obj = ArrayGet(&gObjects, i);
			if (obj->postPhysics) {
				obj->postPhysics(obj);
			}
		}
		///// END OF PHYSICS /////

		///// GRAPHICS /////
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderClear(gRenderer);
		// Draw terrain first
		Object* terrain = ArrayGet(&gObjects, 0);
		if (terrain->ovrdGraphics) {
			terrain->ovrdGraphics(terrain);
		}
		DrawListSort(&gDrawList);
		for (size_t i = 0; i < DrawListLength(&gDrawList); i++) {
			Object *obj = DrawListGet(&gDrawList, i);
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

		unsigned end_ticks = SDL_GetTicks();
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

Array* CurrentObjectArray() {
	return &gObjects;
}

DrawList* CurrentDrawList() {
	return &gDrawList;
}
