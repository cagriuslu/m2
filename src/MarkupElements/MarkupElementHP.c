#include "../MarkupElement.h"
#include "../Game.h"
#include "../Object.h"
#include <stdio.h>

XErr MarkupElementHP_UpdateDynamicText(MarkupElementState* state) {
	if (state->textTexture) {
		SDL_DestroyTexture(state->textTexture);
		state->textTexture = NULL;
	}
	
	// Lookup player's health
	ID playerId = GAME->playerId;
	Object* playerObj = Pool_GetById(&GAME->objects, playerId);
	if (playerObj) {
		ComponentDefense* def = Pool_GetById(&GAME->defenses, playerObj->defense);
		if (def) {
			char buffer[64] = { 0 };
			snprintf(buffer, sizeof(buffer), "%d", (int)roundf(def->hp));
			state->textTexture = Markup_GenerateFontTexture(buffer);
		}
	}
	
	return XOK;
}
