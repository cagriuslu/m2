#include "UiButton.h"
#include "../Main.h"
#include "../Error.h"
#include <SDL.h>
#include <assert.h>

#define AsUiButtonData(uiButtonData) ((UiButtonData*) (uiButtonData))

typedef struct _UiButtonData {
	Vec2I size;
	int alignment;
	SDL_Texture *textTexture;
	int textW, textH;
} UiButtonData;

void UiButton_onMouseButton(Ui* ui) {

}

void UiButton_draw(Ui* ui) {
	Vec2I size = AsUiButtonData(ui->privData)->size;
	SDL_Rect outlineRect = (SDL_Rect) {
		CurrentScreenWidth() / 2 - size.x / 2,
		CurrentScreenHeight() / 2 - size.y / 2,
		size.x,
		size.y
	};

	SDL_SetRenderDrawColor(CurrentRenderer(), 0, 0, 0, 255);
	SDL_RenderFillRect(CurrentRenderer(), &outlineRect);
	SDL_SetRenderDrawColor(CurrentRenderer(), 255, 255, 255, 255);
	SDL_RenderDrawRect(CurrentRenderer(), &outlineRect);

	int textW = AsUiButtonData(ui->privData)->textW, textH = AsUiButtonData(ui->privData)->textH;
	SDL_Rect textRect = (SDL_Rect) {
		CurrentScreenWidth() / 2 - textW / 2,
		CurrentScreenHeight() / 2 - textH / 2,
		textW,
		textH
	};
	SDL_RenderCopy(CurrentRenderer(), AsUiButtonData(ui->privData)->textTexture, NULL, &textRect);
}

int UiButtonInit(Ui *ui, Vec2I size, int alignment, const char *text) {
	PROPAGATE_ERROR(UiInit(ui));
	ui->onMouseButton = UiButton_onMouseButton;
	ui->draw = UiButton_draw;
	ui->privData = malloc(sizeof(UiButtonData));
	assert(ui->privData);
	AsUiButtonData(ui->privData)->size = size;
	AsUiButtonData(ui->privData)->alignment = alignment;

	SDL_Surface *textSurface = TTF_RenderUTF8_Blended(CurrentFont(), text, (SDL_Color) {255, 255, 255, 255});
	SDL_Texture *textTexture = SDL_CreateTextureFromSurface(CurrentRenderer(), textSurface);
	AsUiButtonData(ui->privData)->textTexture = textTexture;
	AsUiButtonData(ui->privData)->textW = textSurface->w;
	AsUiButtonData(ui->privData)->textH = textSurface->h;
	SDL_FreeSurface(textSurface);
	return 0;
}

void UiButtonDeinit(Ui *ui) {

}
