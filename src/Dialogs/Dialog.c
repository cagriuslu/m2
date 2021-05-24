#include "Dialog.h"
#include "../EventHandling.h"
#include "../Main.h"
#include "../Error.h"
#include "../Uis/Ui.h"
#include "../Debug.h"
#include <stdbool.h>

int DialogWaitForEvent(Array *uis) {
	while (true) {
		///// EVENT HANDLING /////
		bool quit = false;
		bool button = false;
		GatherEvents(&quit, NULL, NULL, NULL, &button, NULL);
		if (quit) {
			return X_QUIT;
		}
		if (button) {
			bool eventOccured = false;
			for (size_t i = 0; i < ArrayLength(uis); i++) {
				Ui *ui = ArrayGet(uis, i);
				if (ui->onMouseButton) {
					// Check if mouse is on top of Ui
					Vec2I poiPos = PointerPosition();
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
		///// END OF EVENT HANDLING /////

		///// GRAPHICS /////
		SDL_SetRenderDrawColor(CurrentRenderer(), 0, 0, 0, 255);
		SDL_RenderClear(CurrentRenderer());
		for (size_t i = 0; i < ArrayLength(uis); i++) {
			Ui *ui = ArrayGet(uis, i);
			if (ui->draw) {
				ui->draw(ui);
			}
		}
		SDL_RenderPresent(CurrentRenderer());
		///// END OF GRAPHICS /////
	}
}
