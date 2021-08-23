#include "Hud.h"
#include "Main.h"
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
		
		return;
	} else if (level->levelType == LEVEL_TYPE_LEVEL_EDITOR) {
		return;
	}
}
