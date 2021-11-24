#include "Game.h"

void Game_SetWidthHeight(Game* game, int width, int height) {
	float fw = (float)width;
	float fh = (float)height;
	
	game->windowWidth = width;
	game->windowHeight = height;
	float aspectRatioDiff = (fw / fh) - GAME_AND_HUD_ASPECT_RATIO;
	if (0.001f < aspectRatioDiff) {
		// Screen is wider than expected, we need envelope on left & right
		game->gameWidth = (int)roundf(fh * GAME_ASPECT_RATIO);
		game->gameHeight = height;
		game->gameAndHudWidth = (int)roundf(fh * GAME_AND_HUD_ASPECT_RATIO);
		game->gameAndHudHeight = height;
		int envelopeWidth = (width - game->gameAndHudWidth) / 2;
		game->firstEnvelopeRect = (SDL_Rect){ 0, 0, envelopeWidth, height };
		game->secondEnvelopeRect = (SDL_Rect){ width - envelopeWidth, 0, envelopeWidth, height };
		int hudWidth = (int)roundf((float)game->gameAndHudHeight * HUD_ASPECT_RATIO);
		game->leftHudRect = (SDL_Rect){ envelopeWidth, 0, hudWidth, game->gameAndHudHeight };
		game->rightHudRect = (SDL_Rect){ width - envelopeWidth - hudWidth, 0, hudWidth, game->gameAndHudHeight };
	} else if (aspectRatioDiff < -0.001f) {
		// Screen is taller than expected, we need envelope on top & bottom
		game->gameWidth = width;
		game->gameHeight = (int)roundf(fw / GAME_ASPECT_RATIO);
		game->gameAndHudWidth = width;
		game->gameAndHudHeight = (int)roundf(fw / GAME_AND_HUD_ASPECT_RATIO);
		int envelopeWidth = (height - game->gameAndHudHeight) / 2;
		game->firstEnvelopeRect = (SDL_Rect){ 0, 0, width, envelopeWidth };
		game->secondEnvelopeRect = (SDL_Rect){ 0, height - envelopeWidth, width, envelopeWidth };
		int hudWidth = (int)roundf((float)game->gameAndHudHeight * HUD_ASPECT_RATIO);
		game->leftHudRect = (SDL_Rect){ 0, envelopeWidth, hudWidth, game->gameAndHudHeight };
		game->rightHudRect = (SDL_Rect){ width - hudWidth, envelopeWidth, hudWidth, game->gameAndHudHeight };
	} else {
		game->gameWidth = width;
		game->gameHeight = height;
		game->gameAndHudWidth = width;
		game->gameAndHudHeight = height;
		game->firstEnvelopeRect = (SDL_Rect){ 0,0,0,0, };
		game->secondEnvelopeRect = (SDL_Rect){ 0,0,0,0, };
		int hudWidth = (int)roundf((float)game->gameAndHudHeight * HUD_ASPECT_RATIO);
		game->leftHudRect = (SDL_Rect){ 0, 0, hudWidth, game->gameAndHudHeight };
		game->rightHudRect = (SDL_Rect){ width - hudWidth, 0, hudWidth, game->gameAndHudHeight };
	}
	game->pixelsPerMeter = (float)game->gameAndHudHeight / 16.0f;
}
