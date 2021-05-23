#include "UIButton.h"
#include "../Main.h"
#include "../Error.h"
#include <SDL.h>
#include <assert.h>

#define AsUIButtonData(uiButtonData) ((UIButtonData*) (uiButtonData))

typedef struct _UIButtonData {
	Vec2I size;
	int alignment;
	SDL_Texture *textTexture;
	int textW, textH;
} UIButtonData;

void UIButton_onMouseButton(UI* ui) {

}

void UIButton_draw(UI* ui) {
	Vec2I size = AsUIButtonData(ui->privData)->size;
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

	int textW = AsUIButtonData(ui->privData)->textW, textH = AsUIButtonData(ui->privData)->textH;
	SDL_Rect textRect = (SDL_Rect) {
		CurrentScreenWidth() / 2 - textW / 2,
		CurrentScreenHeight() / 2 - textH / 2,
		textW,
		textH
	};
	SDL_RenderCopy(CurrentRenderer(), AsUIButtonData(ui->privData)->textTexture, NULL, &textRect);
}

int UIButtonInit(UI *ui, Vec2I size, int alignment, const char *text) {
	PROPAGATE_ERROR(UIInit(ui));
	ui->onMouseButton = UIButton_onMouseButton;
	ui->draw = UIButton_draw;
	ui->privData = malloc(sizeof(UIButtonData));
	assert(ui->privData);
	AsUIButtonData(ui->privData)->size = size;
	AsUIButtonData(ui->privData)->alignment = alignment;

	SDL_Surface *textSurface = TTF_RenderUTF8_Blended(CurrentFont(), text, (SDL_Color) {255, 255, 255, 255});
	SDL_Texture *textTexture = SDL_CreateTextureFromSurface(CurrentRenderer(), textSurface);
	AsUIButtonData(ui->privData)->textTexture = textTexture;
	AsUIButtonData(ui->privData)->textW = textSurface->w;
	AsUIButtonData(ui->privData)->textH = textSurface->h;
	SDL_FreeSurface(textSurface);
	return 0;
}

void UIButtonDeinit(UI *ui) {

}
