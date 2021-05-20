#include "Main.h"
#include "Array.h"
#include "Player.h"
#include "Camera.h"
#include "Vec2I.h"
#include "Debug.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

Array gObjects;
uint8_t gKeysPressed[_KEY_COUNT];
uint8_t gKeysReleased[_KEY_COUNT];
uint8_t gKeysState[_KEY_COUNT];

int main(int argc, char *argv[]) {
	const int SCREEN_WIDTH = 640;
	const int SCREEN_HEIGHT = 480;
	const int SCREEN_HALF_WIDTH = SCREEN_WIDTH / 2;
	const int SCREEN_HALF_HEIGHT = SCREEN_HEIGHT / 2;
	const float PIXELS_PER_METER = 20.0;

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	SDL_Window *window = SDL_CreateWindow("cgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	ArrayInit(&gObjects, sizeof(Object));
	Object *player = ArrayAppend(&gObjects, NULL); // Append empty Player object
	PlayerInit(player);
	Object *camera = ArrayAppend(&gObjects, NULL); // Append empty Camera object
	CameraInit(camera, player);

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

		for (size_t i = 0; i < ArrayLength(&gObjects); i++) {
			Object *obj = ArrayGet(&gObjects, i);
			if (obj->prePhysics) {
				obj->prePhysics(obj);
			}
		}
		// Physics
		for (size_t i = 0; i < ArrayLength(&gObjects); i++) {
			Object *obj = ArrayGet(&gObjects, i);
			if (obj->postPhysics) {
				obj->postPhysics(obj);
			}
		}

		// Graphics
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		for (size_t i = 0; i < ArrayLength(&gObjects); i++) {
			Object *obj = ArrayGet(&gObjects, i);
			if (obj->preGraphics) {
				obj->preGraphics(obj);
			}
			if (obj->ovrdGraphics) {
				Vec3F obj_origin_wrt_camera_obj = Vec3FSub(obj->pos, camera->pos);
				Vec2I obj_origin_wrt_screen_center = Vec3Fto2I(Vec3FMul(obj_origin_wrt_camera_obj, PIXELS_PER_METER));
				Vec2I obj_gfx_origin_wrt_screen_center = Vec2IAdd(obj_origin_wrt_screen_center, obj->txOff);
				Vec2I obj_gfx_origin_wrt_screen_origin = Vec2IAdd((Vec2I) {SCREEN_HALF_WIDTH, SCREEN_HALF_HEIGHT}, obj_gfx_origin_wrt_screen_center);
				SDL_Rect viewport = (SDL_Rect) {
					obj_gfx_origin_wrt_screen_origin.x - (int32_t) round(obj->txSrc.w * obj->txScaleW / 2.0), 
					obj_gfx_origin_wrt_screen_origin.y - (int32_t) round(obj->txSrc.h * obj->txScaleH / 2.0),
					(int32_t) round(obj->txSrc.w * obj->txScaleW),
					(int32_t) round(obj->txSrc.h * obj->txScaleH)
				};
				SDL_RenderSetViewport(renderer, &viewport);
				obj->ovrdGraphics(obj, renderer);
			}
			if (obj->postGraphics) {
				obj->postGraphics(obj);
			}
		}
		SDL_RenderPresent(renderer);

		unsigned end_ticks = SDL_GetTicks();
		//fprintf(stderr, "Frame time: %u\n", end_ticks - start_ticks);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
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
