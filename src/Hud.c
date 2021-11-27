#include "Hud.h"
#include "Main.h"
#include "Object.h"
#include "SDLUtils.h"
#include <string.h>

#define AS_GAME(ptr) ((Game*)(ptr))

XErr Hud_Init(Hud* hud) {
	memset(hud, 0, sizeof(Hud));
	// TODO
	return 0;
}

void Hud_Term(Hud* hud) {
	memset(hud, 0, sizeof(Hud));
}

void Hud_Draw(Hud* hud) {
	(void)hud;
	int paddingSize = (int)roundf((float)GAME->leftHudRect.w / 12.0f);
	
	SDL_Rect leftHudDrawingArea = SDLUtils_ShrinkRect(GAME->leftHudRect, paddingSize, paddingSize);
	SDL_Rect rightHudDrawingArea = SDLUtils_ShrinkRect(GAME->rightHudRect, paddingSize, paddingSize);
	//SDLUtils_SplitRect(GAME->leftHudRect, 10, 10, 1, 8, 1, 8);

	SDL_SetRenderDrawColor(GAME->sdlRenderer, 50, 50, 50, 255);
	SDL_RenderFillRect(GAME->sdlRenderer, &leftHudDrawingArea);
	SDL_RenderFillRect(GAME->sdlRenderer, &rightHudDrawingArea);
	
	if (GAME->levelType == LEVEL_TYPE_SINGLE_PLAYER) {
		// Draw player health
		Object* player = (Object*)Pool_GetById(&GAME->objects, GAME->playerId);
		
		if (player) {
			ComponentDefense* defense = FindDefenseOfObject(player);
			if (defense) {
				SDL_Rect hpBorderRect = SDLUtils_SplitRect(leftHudDrawingArea, 1, 4, 0, 1, 3, 1);
				SDL_SetRenderDrawColor(GAME->sdlRenderer, 255, 255, 255, 255);
				SDL_RenderFillRect(GAME->sdlRenderer, &hpBorderRect);
				SDL_Rect hpPaddingRect = SDLUtils_ShrinkRect(hpBorderRect, 2, 2);
				SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
				SDL_RenderFillRect(GAME->sdlRenderer, &hpPaddingRect);
				SDL_Rect hpRect = SDLUtils_ShrinkRect2(hpPaddingRect, 2 + ((hpPaddingRect.h - 4) - (hpPaddingRect.h - 4) * defense->hp / defense->maxHp), 2, 2, 2);
				SDL_SetRenderDrawColor(GAME->sdlRenderer, 255, 0, 0, 255);
				SDL_RenderFillRect(GAME->sdlRenderer, &hpRect);
			}
		}
		
		return;
	} else if (GAME->levelType == LEVEL_TYPE_LEVEL_EDITOR) {
		return;
	}
}
