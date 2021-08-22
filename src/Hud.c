#include "Hud.h"
#include "Main.h"
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
	
	
	Level* level = CurrentLevel();
	if (level->levelType == LEVEL_TYPE_SINGLE_PLAYER) {
		// Draw player health
		
		return;
	} else if (level->levelType == LEVEL_TYPE_LEVEL_EDITOR) {
		return;
	}
}
