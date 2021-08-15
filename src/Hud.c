#include "Hud.h"
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
	
}
