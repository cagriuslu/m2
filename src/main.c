#include "Main.h"
#include "Objects/StaticBox.h"
#include "Box2DWrapper.h"
#include "Array.h"
#include "Player.h"
#include "Camera.h"
#include "Vec2I.h"
#include "UI.h"
#include "Debug.h"
#include "UIs/UIPanel.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

int gScreenWidth = 640, gScreenHeight = 480;
SDL_Renderer *gRenderer;
SDL_Texture *gTextureLUT;
Box2DWorld *gWorld;
Array gObjects;
Array gUIs;
uint8_t gKeysPressed[_KEY_COUNT];
uint8_t gKeysReleased[_KEY_COUNT];
uint8_t gKeysState[_KEY_COUNT];

int main(int argc, char *argv[]) {
	const int SCREEN_HALF_WIDTH = gScreenWidth / 2;
	const int SCREEN_HALF_HEIGHT = gScreenHeight / 2;
	const float PIXELS_PER_METER = 40.0;
	const float timeStep = 1.0 / 60.0;
	const int velocityIterations = 8;
	const int positionIterations = 3;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	SDL_Window *window = SDL_CreateWindow("cgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gScreenWidth, gScreenHeight, SDL_WINDOW_SHOWN);
	gRenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	gTextureLUT = SDL_CreateTextureFromSurface(gRenderer, IMG_Load("16x16.png"));
	gWorld = Box2DWorldCreate((Vec2F) {0.0, 0.0});
	ArrayInit(&gObjects, sizeof(Object));
	ArrayInit(&gUIs, sizeof(UI));

	Object *player = ArrayAppend(&gObjects, NULL); // Append empty Player object
	PlayerInit(player);
	Object *camera = ArrayAppend(&gObjects, NULL); // Append empty Camera object
	CameraInit(camera, player);

	// Test object
	Object *staticBox1 = ArrayAppend(&gObjects, NULL);
	StaticBoxInit(staticBox1, (Vec2F) {5.0, 5.0});

	// Test panel
	UI *panel = ArrayAppend(&gUIs, NULL);
	UIPanelInit(panel, (Vec2I) {200, 200});

	bool quit = false;
	while (!quit) {
		unsigned start_ticks = SDL_GetTicks();

		// Clear events
		memset(gKeysPressed, 0, sizeof(gKeysPressed));
		memset(gKeysReleased, 0, sizeof(gKeysReleased));
		memset(gKeysState, 0, sizeof(gKeysState));
		// Handle events
		SDL_Event e;
		while (!quit && SDL_PollEvent(&e) != 0) {
			switch (e.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_WINDOWEVENT:
				//res = on_event_window(delta_time, &e.window);
				break;
			case SDL_KEYDOWN:
				if (e.key.repeat == 0) {
					gKeysPressed[KeyFromSDLScancode(e.key.keysym.scancode)] = 1;
				}
				break;
			case SDL_KEYUP:
				if (e.key.repeat == 0) {
					gKeysReleased[KeyFromSDLScancode(e.key.keysym.scancode)] = 1;
				}
				break;
			case SDL_MOUSEMOTION:
				//res = on_event_mouse_motion(delta_time, &e.motion);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				//res = on_event_mouse_button(delta_time, &e.button);
				break;
			case SDL_MOUSEWHEEL:
				//res = on_event_mouse_wheel(delta_time, &e.wheel);
				break;
			default:
				break;
			}
		}
		if (quit) {
			break;
		}
		KeyStateArrayFillFromSDLKeyboardStateArray(gKeysState, SDL_GetKeyboardState(NULL));

		// Physics
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
			if (obj->postPhysics) {
				obj->postPhysics(obj);
			}
		}

		// Graphics
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 255);
		SDL_RenderClear(gRenderer);
		for (size_t i = 0; i < ArrayLength(&gObjects); i++) {
			Object *obj = ArrayGet(&gObjects, i);
			if (obj->preGraphics) {
				obj->preGraphics(obj);
			}
			if (obj->ovrdGraphics) {
				obj->ovrdGraphics(obj);
			} else {
				Vec2F obj_origin_wrt_camera_obj = Vec2FSub(obj->pos, camera->pos);
				Vec2I obj_origin_wrt_screen_center = Vec2Fto2I(Vec2FMul(obj_origin_wrt_camera_obj, PIXELS_PER_METER));
				Vec2I obj_gfx_origin_wrt_screen_center = Vec2IAdd(obj_origin_wrt_screen_center, obj->txOffset);
				Vec2I obj_gfx_origin_wrt_screen_origin = Vec2IAdd((Vec2I) {SCREEN_HALF_WIDTH, SCREEN_HALF_HEIGHT}, obj_gfx_origin_wrt_screen_center);
				int obj_width_on_screen = round(obj->txSize.x) * PIXELS_PER_METER;
				int obj_height_on_screen = round(obj->txSize.y) * PIXELS_PER_METER;
				SDL_Rect dstrect = (SDL_Rect) {
					obj_gfx_origin_wrt_screen_origin.x - obj_width_on_screen / 2, 
					obj_gfx_origin_wrt_screen_origin.y - obj_height_on_screen / 2,
					obj_width_on_screen,
					obj_height_on_screen
				};
				SDL_RenderCopyEx(gRenderer, gTextureLUT, &obj->txSrc, &dstrect, obj->angle, NULL, SDL_FLIP_NONE);
			}
			if (obj->postGraphics) {
				obj->postGraphics(obj);
			}
		}
		for (size_t i = 0; i < ArrayLength(&gUIs); i++) {
			UI *ui = ArrayGet(&gUIs, i);
			if (ui->draw) {
				ui->draw(ui);
			}
		}
		SDL_RenderPresent(gRenderer);

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

SDL_Renderer* CurrentRenderer() {
	return gRenderer;
}

SDL_Texture* CurrentTextureLUT() {
	return gTextureLUT;
}

Box2DWorld* CurrentWorld() {
	return gWorld;
}

bool IsKeyPressed(Key key) {
	return gKeysPressed[key];
}

bool IsKeyReleased(Key key) {
	return gKeysReleased[key];
}

bool IsKeyDown(Key key) {
	return gKeysState[key];
}
