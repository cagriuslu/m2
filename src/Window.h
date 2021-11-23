#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>

#define GAME_AND_HUD_ASPECT_RATIO (16.0f / 9.0f)
#define GAME_ASPECT_RATIO (5.0f / 4.0f)
#define HUD_ASPECT_RATIO ((GAME_AND_HUD_ASPECT_RATIO - GAME_ASPECT_RATIO) / 2.0f)

typedef struct _Window {
	SDL_Window *sdlWindow;
	SDL_Cursor *cursor;
	uint32_t pixelFormat;

	int windowWidth;
	int windowHeight;
	int gameWidth;
	int gameHeight;
	int gameAndHudWidth;
	int gameAndHudHeight;
	SDL_Rect firstEnvelopeRect;
	SDL_Rect secondEnvelopeRect;
	SDL_Rect leftHudRect;
	SDL_Rect rightHudRect;
	float pixelsPerMeter;
} Window;

void Window_SetWidthHeight(Window* window, int width, int height);

#endif
