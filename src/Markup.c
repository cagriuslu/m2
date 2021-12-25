#include "Markup.h"
#include "Event.h"
#include "HashMap.h"
#include "Game.h"

// Store only STATIC STRINGS in this pool
HashMap gMarkupFontTexturePool; // const char* -> SDL_Texture*
void Markup_FontTexturePool_ItemTerminator(void *itemPtr) {
	SDL_Texture *item = *((SDL_Texture**)itemPtr);
	SDL_DestroyTexture(item);
}
XErr Markup_FontTexturePool_Init() {
	return HashMap_Init(&gMarkupFontTexturePool, sizeof(SDL_Texture*), Markup_FontTexturePool_ItemTerminator);
}

SDL_Rect Markup_CalculateElementRect(SDL_Rect parentRect, unsigned parentW, unsigned parentH, unsigned childX, unsigned childY, unsigned childW, unsigned childH) {
	float pixelsPreUnitW = (float)parentRect.w / (float)parentW;
	float pixelsPreUnitH = (float)parentRect.h / (float)parentH;
	return (SDL_Rect){
		parentRect.x + (int)roundf((float)childX * pixelsPreUnitW),
		parentRect.y + (int)roundf((float)childY * pixelsPreUnitH),
		(int)roundf((float)childW * pixelsPreUnitW),
		(int)roundf((float)childH * pixelsPreUnitH)
	};
}

SDL_Texture* Markup_GetFontTexture(const char *text) {
	void *itemPtr = HashMap_GetInt64Key(&gMarkupFontTexturePool, (intptr_t)text);
	if (itemPtr) {
		return *((SDL_Texture**)itemPtr);
	} else {
		// Create texture
		SDL_Surface *textSurf = TTF_RenderUTF8_Blended(GAME->ttfFont, text, (SDL_Color){255, 255, 255, 255});
		SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textSurf);
		SDL_FreeSurface(textSurf);
		// Store in pool
		HashMap_SetInt64Key(&gMarkupFontTexturePool, (intptr_t)text, &texture);
		return texture;
	}
}

XErr Markup_DrawText_NoPresent(SDL_Rect rect, const char *text) {
	SDL_Texture *texture = Markup_GetFontTexture(text);
	int textW = 0, textH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);
	SDL_Rect textRect = (SDL_Rect) {
		rect.x + rect.w / 2 - textW / 2,
		rect.y + rect.h / 2 - textH / 2,
		textW,
		textH
	};
	SDL_RenderCopy(GAME->sdlRenderer, texture, NULL, &textRect);
	return XOK;
}

XErr Markup_Draw_NoPresent(const CfgMarkup *markup, SDL_Rect rect) {
	// Clear rect
	SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
	SDL_RenderFillRect(GAME->sdlRenderer, &rect);
	// Draw elements
	for (const CfgMarkupElement *element = markup ? markup->firstElement : NULL; element; element = element->next) {
		SDL_Rect elementRect = Markup_CalculateElementRect(rect, markup->w, markup->h, element->x, element->y, element->w, element->h);
		// Clear rect
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
		SDL_RenderFillRect(GAME->sdlRenderer, &elementRect);
		// Draw element
		switch (element->type) {
			case CFG_MARKUP_ELEMENT_TYPE_MARKUP:
				Markup_Draw_NoPresent(element->elementUnion.markup.markup, elementRect);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT:
				Markup_DrawText_NoPresent(elementRect, element->elementUnion.staticText.text);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
				Markup_DrawText_NoPresent(elementRect, element->elementUnion.staticTextButton.text);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE:
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT:
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE:
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
				break;
		}
		// Draw element border
		if (element->borderWidth_px) {
			SDL_SetRenderDrawColor(GAME->sdlRenderer, 255, 255, 255, 255);
			SDL_RenderDrawRect(GAME->sdlRenderer, &elementRect);
		}
	}
	// Draw markup border
	if (markup && markup->borderWidth_px) {
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(GAME->sdlRenderer, &rect);
	}
	return XOK;
}

XErr Markup_ExecuteBlocking(const CfgMarkup *markup) {
	Events evs;
	const CfgMarkupElement* buttonDepressedMarkupElement = NULL;
	while (true) {
		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (Events_Gather(&evs)) {
			if (evs.quitEvent) {
				break;
			}
			if (evs.windowResizeEvent) {
				Game_UpdateWindowDimensions(evs.windowDims.x, evs.windowDims.y);
			}
		}
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Clear screen
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
		SDL_RenderClear(GAME->sdlRenderer);
		// Draw markup
		Markup_Draw_NoPresent(markup, (SDL_Rect){0, 0, GAME->windowWidth, GAME->windowHeight});
		// Present
		SDL_RenderPresent(GAME->sdlRenderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////
	}
	return XOK;
}
