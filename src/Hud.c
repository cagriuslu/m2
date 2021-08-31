#include "Hud.h"
#include "Main.h"
#include "Object.h"
#include "SDLUtils.h"
#include <string.h>

XErr Hud_Init(Hud* hud, ID playerId) {
	memset(hud, 0, sizeof(Hud));
	hud->playerId = playerId;
	return 0;
}

void Hud_Term(Hud* hud) {
	memset(hud, 0, sizeof(Hud));
}

void Hud_Draw(Hud* hud) {
	int paddingSize = (int)roundf((float)CurrentWindow()->leftHudRect.w / 12.0f);
	
	SDL_Rect leftHudDrawingArea = SDLUtils_ShrinkRect(CurrentWindow()->leftHudRect, paddingSize, paddingSize);
	SDL_Rect rightHudDrawingArea = SDLUtils_ShrinkRect(CurrentWindow()->rightHudRect, paddingSize, paddingSize);
	//SDLUtils_SplitRect(CurrentWindow()->leftHudRect, 10, 10, 1, 8, 1, 8);

	SDL_SetRenderDrawColor(CurrentRenderer(), 50, 50, 50, 255);
	SDL_RenderFillRect(CurrentRenderer(), &leftHudDrawingArea);
	SDL_RenderFillRect(CurrentRenderer(), &rightHudDrawingArea);
	
	Level* level = CurrentLevel();
	if (level->levelType == LEVEL_TYPE_SINGLE_PLAYER) {
		// Draw player health
		Object* player = (Object*)Pool_GetById(&level->objects, hud->playerId);
		if (player) {
			ComponentDefense* defense = FindDefenseOfObject(player);
			if (defense) {
				SDL_Rect hpBorderRect = SDLUtils_SplitRect(leftHudDrawingArea, 1, 4, 0, 1, 3, 1);
				SDL_SetRenderDrawColor(CurrentRenderer(), 255, 255, 255, 255);
				SDL_RenderFillRect(CurrentRenderer(), &hpBorderRect);
				SDL_Rect hpPaddingRect = SDLUtils_ShrinkRect(hpBorderRect, 2, 2);
				SDL_SetRenderDrawColor(CurrentRenderer(), 0, 0, 0, 255);
				SDL_RenderFillRect(CurrentRenderer(), &hpPaddingRect);
				SDL_Rect hpRect = SDLUtils_ShrinkRect2(hpPaddingRect, 2 + ((hpPaddingRect.h - 4) - (hpPaddingRect.h - 4) * defense->hp / defense->maxHp), 2, 2, 2);
				SDL_SetRenderDrawColor(CurrentRenderer(), 255, 0, 0, 255);
				SDL_RenderFillRect(CurrentRenderer(), &hpRect);
			}
		}
		
		return;
	} else if (level->levelType == LEVEL_TYPE_LEVEL_EDITOR) {
		return;
	}
}
