#include "Player.h"
#include "Camera.h"
#include "Array.h"
#include "Vec2I.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

int main(int argc, char *argv[]) {
	const int SCREEN_WIDTH = 640;
	const int SCREEN_HEIGHT = 480;
	const int SCREEN_HALF_WIDTH = SCREEN_WIDTH / 2;
	const int SCREEN_HALF_HEIGHT = SCREEN_HEIGHT / 2;
	const float PIXELS_PER_METER = 20.0;

	int res = 0;

	res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
	fprintf(stderr, "SDL_Init: %u\n", res);
	res = IMG_Init(IMG_INIT_PNG);
	fprintf(stderr, "IMG_Init: %u\n", res);
	res = TTF_Init();
	fprintf(stderr, "TTF_Init: %u\n", res);
	SDL_Window *window = SDL_CreateWindow("cgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	uint32_t pixel_format = SDL_GetWindowPixelFormat(window);
	fprintf(stderr, "Pixel format: %x\n", pixel_format);
	fprintf(stderr, "SDL_CreateWindow: %p\n", window);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	fprintf(stderr, "SDL_CreateRenderer: %p\n", renderer);
	SDL_RendererInfo renderer_info;
	SDL_GetRendererInfo(renderer, &renderer_info);
	fprintf(stderr, "Renderer name: %s\n", renderer_info.name);
	fprintf(stderr, "SDL_RENDERER_SOFTWARE enabled: %d\n", renderer_info.flags & SDL_RENDERER_SOFTWARE ? 1 : 0);
	fprintf(stderr, "SDL_RENDERER_ACCELERATED enabled: %d\n", renderer_info.flags & SDL_RENDERER_ACCELERATED ? 1 : 0);
	fprintf(stderr, "SDL_RENDERER_PRESENTVSYNC enabled: %d\n", renderer_info.flags & SDL_RENDERER_PRESENTVSYNC ? 1 : 0);
	fprintf(stderr, "SDL_RENDERER_TARGETTEXTURE enabled: %d\n", renderer_info.flags & SDL_RENDERER_TARGETTEXTURE ? 1 : 0);

	Array objects;
	ArrayInit(&objects, sizeof(Object));
	Object *player = ArrayAppend(&objects, NULL); // Append empty Player object
	PlayerInit(player);
	Object *camera = ArrayAppend(&objects, NULL); // Append empty Camera object
	CameraInit(camera, player);

	bool quit = false;
	while (!quit) {
		unsigned start_ticks = SDL_GetTicks();

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
				case SDL_KEYUP:
					//res = on_event_keyboard(delta_time, &e.key);
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

		// Graphics
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		for (size_t i = 0; i < ArrayLength(&objects); i++) {
			Object *obj = ArrayGet(&objects, i);
			Vec3F obj_origin_wrt_cam = Vec3FSub(obj->pos, camera->pos);
			Vec3F obj_gfx_origin_wrt_cam = Vec3FAdd(obj_origin_wrt_cam, obj->txOff);
			Vec2I obj_gfx_origin_wrt_screen_center = Vec3Fto2I(Vec3FMul(obj_gfx_origin_wrt_cam, PIXELS_PER_METER));
			Vec2I obj_gfx_origin_wrt_screen_origin = Vec2IAdd((Vec2I) {SCREEN_HALF_WIDTH, SCREEN_HALF_HEIGHT}, obj_gfx_origin_wrt_screen_center);
			SDL_Rect viewport = (SDL_Rect) {
				obj_gfx_origin_wrt_screen_origin.x - (int32_t) round(obj->txSrc.w * obj->txScaleW / 2.0), 
				obj_gfx_origin_wrt_screen_origin.y - (int32_t) round(obj->txSrc.h * obj->txScaleH / 2.0),
				(int32_t) round(obj->txSrc.w * obj->txScaleW),
				(int32_t) round(obj->txSrc.h * obj->txScaleH)
			};
			res = SDL_RenderSetViewport(renderer, &viewport);
			if (res != 0) {
				fprintf(stderr, "SDL_RenderSetViewport: %s\n", SDL_GetError());
				abort();
			} 
			obj->ovrdGraphics(obj, renderer);
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
