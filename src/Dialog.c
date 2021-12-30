#include "Dialog.h"
#include "Event.h"
#include "Main.h"
#include "Error.h"
#include "Ui.h"
#include <stdbool.h>

int DialogWaitForEvent(Array *uis) {
	while (true) {
		///// EVENT HANDLING /////
		Events events;
		if (Events_Gather(&events)) {
			if (events.quitEvent) {
				return XEVENT_QUIT;
			}
			if (events.buttonsPressed[BUTTON_PRIMARY]) {
				bool eventOccured = false;
				for (size_t i = 0; i < uis->length; i++) {
					Ui* ui = Array_Get(uis, i);
					if (ui->onMouseButton) {
						// Check if mouse is on top of Ui
						Vec2I poiPos = events.mousePosition;
						if (ui->trigZone.x <= poiPos.x && poiPos.x < ui->trigZone.x + ui->trigZone.w &&
							ui->trigZone.y <= poiPos.y && poiPos.y < ui->trigZone.y + ui->trigZone.h) {
							eventOccured = true;
							ui->onMouseButton(ui);
						}
					}
				}
				if (eventOccured) {
					return X_BUTTON;
				}
			}
		}
		///// END OF EVENT HANDLING /////

		///// GRAPHICS /////
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
		SDL_RenderClear(GAME->sdlRenderer);
		for (size_t i = 0; i < uis->length; i++) {
			Ui *ui = Array_Get(uis, i);
			if (ui->draw) {
				ui->draw(ui);
			}
		}
		SDL_RenderPresent(GAME->sdlRenderer);
		///// END OF GRAPHICS /////
	}
}
