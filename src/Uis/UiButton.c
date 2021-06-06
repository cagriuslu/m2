#include "../Ui.h"
#include "../Main.h"
#include "../Error.h"
#include <SDL.h>
#include <stdarg.h>
#include <assert.h>

#define AsUiButtonData(uiButtonData) ((UiButtonData*) (uiButtonData))

typedef struct _UiButtonData {
	Vec2I position;
	int alignment;
	Vec2I size;
	Vec2I textSize;
	SDL_Texture *texture;
} UiButtonData;

void UiButton_draw(Ui* ui) {
	UiButtonData *uiButtonData = ui->privData;

	Vec2I size = uiButtonData->size;
	SDL_Rect outlineRect = (SDL_Rect) {
		CurrentScreenWidth() / 2 - size.x / 2 + uiButtonData->position.x,
		CurrentScreenHeight() / 2 - size.y / 2 + uiButtonData->position.y,
		size.x,
		size.y
	};
	ui->trigZone = outlineRect;

	SDL_SetRenderDrawColor(CurrentRenderer(), 0, 0, 0, 255);
	SDL_RenderFillRect(CurrentRenderer(), &outlineRect);
	SDL_SetRenderDrawColor(CurrentRenderer(), 255, 255, 255, 255);
	SDL_RenderDrawRect(CurrentRenderer(), &outlineRect);

	int textW = uiButtonData->textSize.x;
	int textH = uiButtonData->textSize.y;
	SDL_Rect textRect = (SDL_Rect) {
		CurrentScreenWidth() / 2 - textW / 2 + uiButtonData->position.x,
		CurrentScreenHeight() / 2 - textH / 2 + uiButtonData->position.y,
		textW,
		textH
	};
	SDL_RenderCopy(CurrentRenderer(), AsUiButtonData(ui->privData)->texture, NULL, &textRect);
}

int UiButtonInit(Ui *ui, Vec2I position, Vec2I minSize, Vec2I pad, int alignment, const char *text) {
	PROPAGATE_ERROR(UiInit(ui));
	
	SDL_Surface *textSurf = TTF_RenderUTF8_Blended(CurrentFont(), text, (SDL_Color) {255, 255, 255, 255});
	Vec2I size = (Vec2I) {MAX(textSurf->w + 2 * pad.x, minSize.x), MAX(textSurf->h + 2 * pad.y, minSize.y)};
	Vec2I textSize = (Vec2I) {textSurf->w, textSurf->h};
	SDL_Texture *texture = SDL_CreateTextureFromSurface(CurrentRenderer(), textSurf);
	SDL_FreeSurface(textSurf);

	UiButtonData *uiButtonData = malloc(sizeof(UiButtonData));
	assert(uiButtonData);
	uiButtonData->position = position;
	uiButtonData->alignment = alignment;
	uiButtonData->size = size;
	uiButtonData->textSize = textSize;
	uiButtonData->texture = texture;

	ui->draw = UiButton_draw;
	ui->privData = uiButtonData;
	return 0;
}

Vec2I UiButtonSize(Ui *ui) {
	UiButtonData *uiButtonData = ui->privData;
	return (Vec2I) {uiButtonData->size.x, uiButtonData->size.y};
}

void UiButtonSetSize(Ui *ui, Vec2I size) {
	UiButtonData *uiButtonData = ui->privData;
	uiButtonData->size = size;
}

void UiButtonDeinit(Ui *ui) {
	(void)ui;
	// TODO
}

Vec2I UiButtonMaxSizeOfButtons(unsigned n, ...) {
    va_list args;
    va_start(args, n);

    Vec2I maxSize = (Vec2I) {0, 0};
    for (unsigned i = 0; i < n; i++) {
    	Ui *ui = va_arg(args, Ui*);
    	maxSize = (Vec2I) {
			MAX(maxSize.x, UiButtonSize(ui).x),
			MAX(maxSize.y, UiButtonSize(ui).y)	
		};
    }

    va_end(args);
    return maxSize;
}
