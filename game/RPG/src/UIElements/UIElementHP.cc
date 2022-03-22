#include "../ARPG_UIElement.hh"
#include "m2/Game.hh"
#include <m2/object/Object.hh>
#include <game/component.hh>
#include <stdio.h>

M2Err UIElementHP_UpdateDynamicText(UIElementState* state) {
	if (state->textTexture) {
		SDL_DestroyTexture(state->textTexture);
		state->textTexture = nullptr;
	}
	
	// Lookup player's health
	auto* player = GAME.objects.get(GAME.playerId);
	if (player) {
		auto& def = GAME.defenses[player->defense_id];
		char buffer[64] = { 0 };
		snprintf(buffer, sizeof(buffer), "%d", (int)roundf(def.hp));
		state->textTexture = UI_GenerateFontTexture(buffer);
	}
	return M2OK;
}
