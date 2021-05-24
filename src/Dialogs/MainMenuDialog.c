#include "MainMenuDialog.h"
#include "../Uis/Ui.h"
#include "../Main.h"
#include "../Uis/UiButton.h"
#include "../EventHandling.h"
#include "Dialog.h"
#include "../Array.h"

#define AsInt(i) ((int*) (i))

void NewGameButton_onMouseButton(Ui *ui) {
	if (IsButtonPressed(BUTTON_PRIMARY)) {
		*AsInt(ui->eventData) = X_MAIN_MENU_NEW_GAME;
	}
}

void LevelEditorButton_onMouseButton(Ui *ui) {
	if (IsButtonPressed(BUTTON_PRIMARY)) {
		*AsInt(ui->eventData) = X_MAIN_MENU_LEVEL_EDITOR;
	}
}

int MainMenuDialog() {
	Array uis;
	ArrayInit(&uis, sizeof(Ui));

	int pressedButton = 0;

	Ui *newGameButton = ArrayAppend(&uis, NULL);
	UiButtonInit(newGameButton, (Vec2I) {0, -35}, (Vec2I) {0, 0}, (Vec2I) {15, 15}, 0, "New Game");
	newGameButton->eventData = &pressedButton;
	newGameButton->onMouseButton = NewGameButton_onMouseButton;
	Ui *levelEditorButton = ArrayAppend(&uis, NULL);
	UiButtonInit(levelEditorButton, (Vec2I) {0, +35}, (Vec2I) {0, 0}, (Vec2I) {15, 15}, 0, "Level Editor");
	levelEditorButton->eventData = &pressedButton;
	levelEditorButton->onMouseButton = LevelEditorButton_onMouseButton;
	
	Vec2I maxButtonSize = UiButtonMaxSizeOfButtons(2, newGameButton, levelEditorButton);
	UIButtonSetSize(newGameButton, maxButtonSize);
	UIButtonSetSize(levelEditorButton, maxButtonSize);

	while (pressedButton == 0) {
		int res = DialogWaitForEvent(&uis);
		PROPAGATE_ERROR(res);
	}

	UiButtonDeinit(levelEditorButton);
	UiButtonDeinit(newGameButton);
	ArrayDeinit(&uis);
	return pressedButton;
}
