#include "../ARPG_UIElement.h"
#include "m2/Game.h"
#include "m2/Object.h"
#include "../ARPG_Component.h"
#include <stdio.h>

M2Err UIElementHP_UpdateDynamicText(UIElementState* state) {
	if (state->textTexture) {
		SDL_DestroyTexture(state->textTexture);
		state->textTexture = NULL;
	}
	
	// Lookup player's health
	ID playerId = GAME->playerId;
	Object* playerObj = Pool_GetById(&GAME->objects, playerId);
	if (playerObj) {
		ComponentDefense* def = Pool_GetById(&GAME->defenses, playerObj->defense);
		ARPG_ComponentDefense *defData = AS_ARPG_COMPONENTDEFENSE(def->data);
		if (defData) {
			char buffer[64] = { 0 };
			snprintf(buffer, sizeof(buffer), "%d", (int)roundf(defData->hp));
			state->textTexture = UI_GenerateFontTexture(buffer);
		}
	}
	
	return M2OK;
}
