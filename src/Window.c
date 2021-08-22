#include "Window.h"

void Window_SetWidthHeight(Window* window, int width, int height) {
	float fw = (float)width;
	float fh = (float)height;
	
	window->windowWidth = width;
	window->windowHeight = height;
	float aspectRatioDiff = (fw / fh) - GAME_AND_HUD_ASPECT_RATIO;
	if (0.001f < aspectRatioDiff) {
		// Screen is wider than expected, we need envelope on left & right
		window->gameAndHudWidth = (int)roundf(fh * GAME_AND_HUD_ASPECT_RATIO);
		window->gameAndHudHeight = height;
		int envelopeWidth = (width - window->gameAndHudWidth) / 2;
		window->firstEnvelopeRect = (SDL_Rect){ 0, 0, envelopeWidth, height };
		window->secondEnvelopeRect = (SDL_Rect){ width - envelopeWidth, 0, envelopeWidth, height };
		int hudWidth = (int)roundf((float)window->gameAndHudHeight * HUD_ASPECT_RATIO);
		window->leftHudRect = (SDL_Rect){ envelopeWidth, 0, hudWidth, window->gameAndHudHeight };
		window->rightHudRect = (SDL_Rect){ width - envelopeWidth - hudWidth, 0, hudWidth, window->gameAndHudHeight };
	} else if (aspectRatioDiff < -0.001f) {
		// Screen is taller than expected, we need envelope on top & bottom
		window->gameAndHudWidth = width;
		window->gameAndHudHeight = (int)roundf(fw / GAME_AND_HUD_ASPECT_RATIO);
		int envelopeWidth = (height - window->gameAndHudHeight) / 2;
		window->firstEnvelopeRect = (SDL_Rect){ 0, 0, width, envelopeWidth };
		window->secondEnvelopeRect = (SDL_Rect){ 0, height - envelopeWidth, width, envelopeWidth };
		int hudWidth = (int)roundf((float)window->gameAndHudHeight * HUD_ASPECT_RATIO);
		window->leftHudRect = (SDL_Rect){ 0, envelopeWidth, hudWidth, window->gameAndHudHeight };
		window->rightHudRect = (SDL_Rect){ width - hudWidth, envelopeWidth, hudWidth, window->gameAndHudHeight };
	} else {
		window->gameAndHudWidth = width;
		window->gameAndHudHeight = height;
		window->firstEnvelopeRect = (SDL_Rect){ 0,0,0,0, };
		window->secondEnvelopeRect = (SDL_Rect){ 0,0,0,0, };
		int hudWidth = (int)roundf((float)window->gameAndHudHeight * HUD_ASPECT_RATIO);
		window->leftHudRect = (SDL_Rect){ 0, 0, hudWidth, window->gameAndHudHeight };
		window->rightHudRect = (SDL_Rect){ width - hudWidth, 0, hudWidth, window->gameAndHudHeight };
	}
	window->pixelsPerMeter = (float)window->gameAndHudHeight / 16.0f;
}
