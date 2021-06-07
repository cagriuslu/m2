#include "../Dialog.h"
#include "../Ui.h"
#include "../Main.h"
#include "../Event.h"
#include "../Array.h"

#define AsInt(i) ((int*) (i))

void ResumeButton_onMouseButton(Ui *ui) {
	if (IsButtonPressed(BUTTON_PRIMARY)) {
		*AsInt(ui->eventData) = X_MAIN_MENU_RESUME;
	}
}

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

int DialogMainMenu(bool levelLoaded) {
	Array uis;
	ArrayInit(&uis, sizeof(Ui), 16, SIZE_MAX);

	int pressedButton = 0;

	Ui *resumeButton = NULL;
	if (levelLoaded) {
		resumeButton = ArrayAppend(&uis, NULL);
		UiButtonInit(resumeButton, (Vec2I) {0, -105}, (Vec2I) {0, 0}, (Vec2I) {15, 15}, 0, "Resume");
		resumeButton->eventData = &pressedButton;
		resumeButton->onMouseButton = ResumeButton_onMouseButton;
	}
	Ui *newGameButton = ArrayAppend(&uis, NULL);
	UiButtonInit(newGameButton, (Vec2I) {0, -35}, (Vec2I) {0, 0}, (Vec2I) {15, 15}, 0, "New Game");
	newGameButton->eventData = &pressedButton;
	newGameButton->onMouseButton = NewGameButton_onMouseButton;
	Ui *levelEditorButton = ArrayAppend(&uis, NULL);
	UiButtonInit(levelEditorButton, (Vec2I) {0, +35}, (Vec2I) {0, 0}, (Vec2I) {15, 15}, 0, "Level Editor");
	levelEditorButton->eventData = &pressedButton;
	levelEditorButton->onMouseButton = LevelEditorButton_onMouseButton;
	
	Vec2I maxButtonSize = levelLoaded ?
		UiButtonMaxSizeOfButtons(3, newGameButton, levelEditorButton, resumeButton) :
		UiButtonMaxSizeOfButtons(2, newGameButton, levelEditorButton);
	if (levelLoaded) {
		UiButtonSetSize(resumeButton, maxButtonSize);
	}
	UiButtonSetSize(newGameButton, maxButtonSize);
	UiButtonSetSize(levelEditorButton, maxButtonSize);

	while (pressedButton == 0) {
		int res = DialogWaitForEvent(&uis);
		PROPAGATE_ERROR(res);
	}

	UiButtonDeinit(levelEditorButton);
	UiButtonDeinit(newGameButton);
	if (levelLoaded) {
		UiButtonDeinit(resumeButton);
	}
	ArrayDeinit(&uis);
	return pressedButton;
}
