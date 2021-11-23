#include "../Dialog.h"
#include "../Ui.h"
#include "../Main.h"
#include "../Event.h"
#include "../Array.h"

#define AsInt(i) ((int*) (i))

void ResumeButton_onMouseButton(Ui *ui) {
	*AsInt(ui->eventData) = X_MAIN_MENU_RESUME;
}

void NewGameButton_onMouseButton(Ui *ui) {
	*AsInt(ui->eventData) = X_MAIN_MENU_NEW_GAME;
}

void LevelEditorButton_onMouseButton(Ui *ui) {
	*AsInt(ui->eventData) = X_MAIN_MENU_LEVEL_EDITOR;
}

int DialogMainMenu(Game *game, bool levelLoaded) {
	Array uis;
	Array_Init(&uis, sizeof(Ui), 16, SIZE_MAX, NULL);

	int pressedButton = 0;

	Ui *resumeButton = NULL;
	if (levelLoaded) {
		resumeButton = Array_Append(&uis, NULL);
		UiButton_Init(resumeButton, (Vec2I) {0, -105}, (Vec2I) {0, 0}, (Vec2I) {15, 15}, 0, "Resume");
		resumeButton->eventData = &pressedButton;
		resumeButton->onMouseButton = ResumeButton_onMouseButton;
	}
	Ui *newGameButton = Array_Append(&uis, NULL);
	UiButton_Init(newGameButton, (Vec2I) {0, -35}, (Vec2I) {0, 0}, (Vec2I) {15, 15}, 0, "New Game");
	newGameButton->eventData = &pressedButton;
	newGameButton->onMouseButton = NewGameButton_onMouseButton;
	Ui *levelEditorButton = Array_Append(&uis, NULL);
	UiButton_Init(levelEditorButton, (Vec2I) {0, +35}, (Vec2I) {0, 0}, (Vec2I) {15, 15}, 0, "Level Editor");
	levelEditorButton->eventData = &pressedButton;
	levelEditorButton->onMouseButton = LevelEditorButton_onMouseButton;
	
	Vec2I maxButtonSize = levelLoaded ?
		UiButton_MaxSizeOfButtons(3, newGameButton, levelEditorButton, resumeButton) :
		UiButton_MaxSizeOfButtons(2, newGameButton, levelEditorButton);
	if (levelLoaded) {
		UiButton_SetSize(resumeButton, maxButtonSize);
	}
	UiButton_SetSize(newGameButton, maxButtonSize);
	UiButton_SetSize(levelEditorButton, maxButtonSize);

	while (pressedButton == 0) {
		int res = DialogWaitForEvent(game, &uis);
		PROPAGATE_ERROR(res);
	}

	UiButton_Term(levelEditorButton);
	UiButton_Term(newGameButton);
	if (levelLoaded) {
		UiButton_Term(resumeButton);
	}
	Array_Term(&uis);
	return pressedButton;
}
