#include "../Ui.h"
#include "../Main.h"
#include "../Game.h"
#include "../Error.h"
#include "../Defs.h"
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
		GAME->windowWidth / 2 - size.x / 2 + uiButtonData->position.x,
		GAME->windowHeight / 2 - size.y / 2 + uiButtonData->position.y,
		size.x,
		size.y
	};
	ui->trigZone = outlineRect;

	SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
	SDL_RenderFillRect(GAME->sdlRenderer, &outlineRect);
	SDL_SetRenderDrawColor(GAME->sdlRenderer, 255, 255, 255, 255);
	SDL_RenderDrawRect(GAME->sdlRenderer, &outlineRect);

	int textW = uiButtonData->textSize.x;
	int textH = uiButtonData->textSize.y;
	SDL_Rect textRect = (SDL_Rect) {
		GAME->windowWidth / 2 - textW / 2 + uiButtonData->position.x,
		GAME->windowHeight / 2 - textH / 2 + uiButtonData->position.y,
		textW,
		textH
	};
	SDL_RenderCopy(GAME->sdlRenderer, AsUiButtonData(ui->privData)->texture, NULL, &textRect);
}

int UiButton_Init(Ui *ui, Vec2I position, Vec2I minSize, Vec2I pad, int alignment, const char *text) {
	PROPAGATE_ERROR(Ui_Init(ui));
	
	SDL_Surface *textSurf = TTF_RenderUTF8_Blended(GAME->ttfFont, text, (SDL_Color) {255, 255, 255, 255});
	Vec2I size = (Vec2I) {MAX(textSurf->w + 2 * pad.x, minSize.x), MAX(textSurf->h + 2 * pad.y, minSize.y)};
	Vec2I textSize = (Vec2I) {textSurf->w, textSurf->h};
	SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textSurf);
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

Vec2I UiButton_Size(Ui *ui) {
	UiButtonData *uiButtonData = ui->privData;
	return (Vec2I) {uiButtonData->size.x, uiButtonData->size.y};
}

void UiButton_SetSize(Ui *ui, Vec2I size) {
	UiButtonData *uiButtonData = ui->privData;
	uiButtonData->size = size;
}

void UiButton_Term(Ui *ui) {
	(void)ui;
	// TODO
}

Vec2I UiButton_MaxSizeOfButtons(unsigned n, ...) {
    va_list args;
    va_start(args, n);

    Vec2I maxSize = (Vec2I) {0, 0};
    for (unsigned i = 0; i < n; i++) {
    	Ui *ui = va_arg(args, Ui*);
    	maxSize = (Vec2I) {
			MAX(maxSize.x, UiButton_Size(ui).x),
			MAX(maxSize.y, UiButton_Size(ui).y)	
		};
    }

    va_end(args);
    return maxSize;
}
