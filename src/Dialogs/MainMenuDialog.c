#include "MainMenuDialog.h"
#include "../Uis/Ui.h"
#include "../Uis/UiButton.h"
#include "../EventHandling.h"
#include "../Main.h"
#include "../Array.h"

int MainMenuDialog() {
	Array uis;
	ArrayInit(&uis, sizeof(Ui));

	Ui *newGameButton = ArrayAppend(&uis, NULL);
	UiButtonInit(newGameButton, (Vec2I) {0, -35}, (Vec2I) {0, 0}, (Vec2I) {15, 15}, 0, "New Game");
	Ui *levelEditorButton = ArrayAppend(&uis, NULL);
	UiButtonInit(levelEditorButton, (Vec2I) {0, +35}, (Vec2I) {0, 0}, (Vec2I) {15, 15}, 0, "Level Editor");
	Vec2I maxButtonSize = (Vec2I) {
		MAX(UIButtonSize(newGameButton).x, UIButtonSize(levelEditorButton).x),
		MAX(UIButtonSize(newGameButton).y, UIButtonSize(levelEditorButton).y)	
	};
	UIButtonSetSize(newGameButton, maxButtonSize);
	UIButtonSetSize(levelEditorButton, maxButtonSize);

	bool quit = false;
	while (!quit) {
		///// EVENT HANDLING /////
		GatherEvents(&quit);
		if (quit) {
			break;
		}
		///// END OF EVENT HANDLING /////

		///// GRAPHICS /////
		SDL_SetRenderDrawColor(CurrentRenderer(), 0, 0, 0, 255);
		SDL_RenderClear(CurrentRenderer());
		for (size_t i = 0; i < ArrayLength(&uis); i++) {
			Ui *ui = ArrayGet(&uis, i);
			if (ui->draw) {
				ui->draw(ui);
			}
		}
		SDL_RenderPresent(CurrentRenderer());
		///// END OF GRAPHICS /////
	}

	UiButtonDeinit(levelEditorButton);
	UiButtonDeinit(newGameButton);
	ArrayDeinit(&uis);
	if (quit) {
		return X_QUIT;
	}
}
