#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	int res = 0;

	int screen_width = 640;
	int screen_height = 480;
	int screen_half_width = screen_width / 2;
	int screen_half_height = screen_height / 2;

	res = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
	fprintf(stderr, "SDL_Init: %u\n", res);
	res = IMG_Init(IMG_INIT_PNG);
	fprintf(stderr, "IMG_Init: %u\n", res);
	res = TTF_Init();
	fprintf(stderr, "TTF_Init: %u\n", res);
	SDL_Window *window = SDL_CreateWindow("cgame", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
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
