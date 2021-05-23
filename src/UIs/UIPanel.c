#include "UIPanel.h"
#include "../Main.h"
#include "../Error.h"
#include <SDL.h>
#include <assert.h>

#define AsUIPanelData(uiPanelData) ((UIPanelData*) (uiPanelData))

typedef struct _UIPanelData {
	Vec2I size;
} UIPanelData;

void UIPanelData_draw(UI *ui) {
	Vec2I size = AsUIPanelData(ui->privData)->size;
	SDL_Rect dstrect = (SDL_Rect) {
		CurrentScreenWidth() / 2 - size.x / 2,
		CurrentScreenHeight() / 2 - size.y / 2,
		size.x,
		size.y
	};

	SDL_SetRenderDrawColor(CurrentRenderer(), 0, 0, 0, 255);
	SDL_RenderFillRect(CurrentRenderer(), &dstrect);
	SDL_SetRenderDrawColor(CurrentRenderer(), 255, 255, 255, 255);
	SDL_RenderDrawRect(CurrentRenderer(), &dstrect);
}

int UIPanelInit(UI *ui, Vec2I size) {
	PROPAGATE_ERROR(UIInit(ui));
	ui->draw = UIPanelData_draw;
	ui->privData = malloc(sizeof(UIPanelData));
	assert(ui->privData);
	AsUIPanelData(ui->privData)->size = size;
	return 0;
}

void UIPanelDeinit(UI *ui) {

}
