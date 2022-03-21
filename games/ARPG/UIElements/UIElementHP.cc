#include "../ARPG_UIElement.hh"
#include "m2/Game.hh"
#include "m2/Object.hh"
#include <game/component.hh>
#include <stdio.h>

M2Err UIElementHP_UpdateDynamicText(UIElementState* state) {
	if (state->textTexture) {
		SDL_DestroyTexture(state->textTexture);
		state->textTexture = NULL;
	}
	
	// Lookup player's health
	ID playerId = GAME.playerId;
	Object* playerObj = GAME.objects.get(playerId);
	if (playerObj) {
		game::component_defense* def = GAME.defenses.get(playerObj->defense);
		if (def) {
			char buffer[64] = { 0 };
			snprintf(buffer, sizeof(buffer), "%d", (int)roundf(def->hp));
			state->textTexture = UI_GenerateFontTexture(buffer);
		}
	}
	
	return M2OK;
}
