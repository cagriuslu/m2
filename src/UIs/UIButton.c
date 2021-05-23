#include "UIButton.h"
#include "../Main.h"
#include "../Error.h"
#include <SDL.h>
#include <assert.h>

#define AsUIButtonData(uiButtonData) ((UIButtonData*) (uiButtonData))

typedef struct _UIButtonData {
	Vec2I size;
	int alignment;
	const char *text;
} UIButtonData;

void UIButton_onMouseButton(UI* ui) {

}

void UIButton_draw(UI* ui) {

}

int UIButtonInit(UI *ui, Vec2I size, int alignment, const char *text) {
	PROPAGATE_ERROR(UIInit(ui));
	ui->onMouseButton = UIButton_onMouseButton;
	ui->draw = UIButton_draw;
	ui->privData = malloc(sizeof(UIButtonData));
	assert(ui->privData);
	AsUIButtonData(ui->privData)->size = size;
	AsUIButtonData(ui->privData)->alignment = alignment;
	AsUIButtonData(ui->privData)->text = text;
	return 0;
}

void UIButtonDeinit(UI *ui) {

}
