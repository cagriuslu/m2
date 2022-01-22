#include "Markup.h"
#include "Event.h"
#include "Def.h"
#include "MarkupElement.h"
#include "HashMap.h"
#include "Game.h"

// Helper functions

SDL_Rect _Markup_CalculateElementRect(SDL_Rect parentRect, unsigned parentW, unsigned parentH, unsigned childX, unsigned childY, unsigned childW, unsigned childH) {
	float pixelsPreUnitW = (float)parentRect.w / (float)parentW;
	float pixelsPreUnitH = (float)parentRect.h / (float)parentH;
	return (SDL_Rect){
			parentRect.x + (int)roundf((float)childX * pixelsPreUnitW),
			parentRect.y + (int)roundf((float)childY * pixelsPreUnitH),
			(int)roundf((float)childW * pixelsPreUnitW),
			(int)roundf((float)childH * pixelsPreUnitH)
	};
}

SDL_Texture* Markup_GenerateFontTexture(const char *text) {
	SDL_Surface *textSurf = TTF_RenderUTF8_Blended(GAME->ttfFont, text, (SDL_Color){255, 255, 255, 255});
	SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textSurf);
	SDL_FreeSurface(textSurf);
	return texture;
}

XErr _Markup_Draw_BackgroundColor(SDL_Rect rect, SDL_Color color) {
	if (color.a == 0) {
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
	} else {
		SDL_SetRenderDrawColor(GAME->sdlRenderer, color.r, color.g, color.b, color.a);
	}
	SDL_RenderFillRect(GAME->sdlRenderer, &rect);
	return XOK;
}
XErr _Markup_Draw_Text(SDL_Texture* texture, SDL_Rect rect) {
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

MarkupElementState* _MarkupState_FindElementByPixel(MarkupState *state, Vec2I mousePosition) {
	MarkupElementState* foundElement = NULL;
	for (MarkupElementState* elementState = state->firstElement; elementState && (foundElement == NULL); elementState = elementState->next) {
		if (elementState->cfg->type == CFG_MARKUP_ELEMENT_TYPE_MARKUP) {
			foundElement = _MarkupState_FindElementByPixel(elementState->child, mousePosition);
		} else {
			SDL_Point p = (SDL_Point){ mousePosition.x, mousePosition.y };
			foundElement = SDL_PointInRect(&p, &elementState->rect) ? elementState : NULL;
		}
	}
	return foundElement;
}

MarkupElementState* _MarkupState_FindElementByKeyboardShortcut(MarkupState* state, const uint8_t* rawKeyboardState) {
	SDL_Scancode keyboardShortcut = 0;
	MarkupElementState* foundElement = NULL;
	for (MarkupElementState* elementState = state->firstElement; elementState && (foundElement == NULL); elementState = elementState->next) {
		switch (elementState->cfg->type) {
			case CFG_MARKUP_ELEMENT_TYPE_MARKUP:
				foundElement = _MarkupState_FindElementByKeyboardShortcut(elementState->child, rawKeyboardState);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
			case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
				keyboardShortcut = elementState->cfg->keyboardShortcut;
				foundElement = keyboardShortcut && rawKeyboardState[keyboardShortcut] ? elementState : NULL;
				break;
			default:
				break;
		}
	}
	return foundElement;
}

void _MarkupState_ResetDepressedButtons(MarkupState *state) {
	for (MarkupElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		switch (elementState->cfg->type) {
			case CFG_MARKUP_ELEMENT_TYPE_MARKUP:
				_MarkupState_ResetDepressedButtons(elementState->child);
				break;
			default:
				elementState->depressed = false;
				break;
		}
	}
}

// MarkupState methods

XErr MarkupState_Init(MarkupState *state, const CfgMarkup* cfg) {
	memset(state, 0, sizeof(MarkupState));
	state->cfg = cfg;
	if (cfg && cfg->firstElement) {
		// Address of the variable to store the MarkupElementState
		MarkupElementState** elementStatePtrAddress = &state->firstElement;
		// Iterate over CfgMarkupElements
		for (const CfgMarkupElement *cfgMarkupElement = cfg->firstElement; cfgMarkupElement; cfgMarkupElement = cfgMarkupElement->next) {
			// Create MarkupElementState
			MarkupElementState* elementState = calloc(1, sizeof(MarkupElementState));
			if (elementState) {
				elementState->cfg = cfgMarkupElement;
				// Create child MarkupState if the type fits
				if (cfgMarkupElement->type == CFG_MARKUP_ELEMENT_TYPE_MARKUP) {
					MarkupState* childMarkupState = calloc(1, sizeof(MarkupState));
					if (childMarkupState) {
						MarkupState_Init(childMarkupState, cfgMarkupElement->child);
						elementState->child = childMarkupState;
					} else {
						return XERR_OUT_OF_MEMORY;
					}
				}
				// Store MarkupElementState back to the address
				*elementStatePtrAddress = elementState;
				elementStatePtrAddress = &elementState->next;
			} else {
				return XERR_OUT_OF_MEMORY;
			}
		}
	}
	return XOK;
}

XErr MarkupState_UpdatePositions(MarkupState *state, SDL_Rect rootRect) {
	state->rect = rootRect;
	for (MarkupElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		// Update rect
		elementState->rect = _Markup_CalculateElementRect(state->rect, state->cfg->w, state->cfg->h, elementState->cfg->x, elementState->cfg->y, elementState->cfg->w, elementState->cfg->h);
		// Update individual element
		switch (elementState->cfg->type) {
			case CFG_MARKUP_ELEMENT_TYPE_MARKUP:
				XERR_REFLECT(MarkupState_UpdatePositions(elementState->child, elementState->rect));
				break;
			default:
				break;
		}
	}
	return XOK;
}

XErr MarkupState_UpdateElements(MarkupState* state) {
	for (MarkupElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		// Update individual element
		switch (elementState->cfg->type) {
			case CFG_MARKUP_ELEMENT_TYPE_MARKUP:
				XERR_REFLECT(MarkupState_UpdateElements(elementState->child));
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT:
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
				if (!elementState->textTexture) {
					elementState->textTexture = Markup_GenerateFontTexture(elementState->cfg->text);
				}
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT:
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE:
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
				MarkupElement_UpdateDynamic(elementState);
				break;
			default:
				break;
		}
	}
	return XOK;
}

bool MarkupState_HandleEvents(MarkupState *state, Events *evs, CfgMarkupButtonType *outPressedButton) {
	bool eventOccurred = false;
	SDL_Point mousePosition = {.x = evs->mousePosition.x, .y = evs->mousePosition.y};

	if (evs->buttonsPressed[BUTTON_PRIMARY] && SDL_PointInRect(&mousePosition, &state->rect)) {
		MarkupElementState* elementUnderMouse = _MarkupState_FindElementByPixel(state, evs->mousePosition);
		if (elementUnderMouse) {
			elementUnderMouse->depressed = true;
		}
	}

	if (evs->buttonsReleased[BUTTON_PRIMARY]) {
		if (SDL_PointInRect(&mousePosition, &state->rect)) {
			MarkupElementState* elementUnderMouse = _MarkupState_FindElementByPixel(state, evs->mousePosition);
			if (elementUnderMouse && elementUnderMouse->depressed) {
				eventOccurred = true;
				if (outPressedButton) {
					*outPressedButton = elementUnderMouse->cfg->buttonType;
				}
			}
		}
		_MarkupState_ResetDepressedButtons(state);
	}

	{
		MarkupElementState* keyboardShortcutPressedElement = _MarkupState_FindElementByKeyboardShortcut(state, evs->rawKeyStates);
		if (keyboardShortcutPressedElement) {
			eventOccurred = true;
			if (outPressedButton) {
				*outPressedButton = keyboardShortcutPressedElement->cfg->buttonType;
			}
		}
	}

	return eventOccurred;
}

XErr MarkupState_Draw(MarkupState *state) {
	_Markup_Draw_BackgroundColor(state->rect, state->cfg->backgroundColor);
	// Draw elements
	for (MarkupElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		_Markup_Draw_BackgroundColor(elementState->rect, elementState->cfg->backgroundColor);
		// Draw element
		switch (elementState->cfg->type) {
			case CFG_MARKUP_ELEMENT_TYPE_MARKUP:
				MarkupState_Draw(elementState->child);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT:
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT:
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
				_Markup_Draw_Text(elementState->textTexture, elementState->rect);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE:
			case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE:
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
				// TODO
				break;
			default:
				break;
		}
		// Draw element border
		if (elementState->cfg->borderWidth_px) {
			if (elementState->depressed == false) {
				SDL_SetRenderDrawColor(GAME->sdlRenderer, 255, 255, 255, 255);
			} else {
				SDL_SetRenderDrawColor(GAME->sdlRenderer, 127, 127, 127, 255);
			}
			SDL_RenderDrawRect(GAME->sdlRenderer, &elementState->rect);
		}
	}
	// Draw markup border
	if (state->cfg->borderWidth_px) {
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(GAME->sdlRenderer, &state->rect);
	}
	return XOK;
}

void MarkupState_Term(MarkupState *state) {
	// TODO
}

XErr Markup_ExecuteBlocking(const CfgMarkup *markup, CfgMarkupButtonType* outPressedButton) {
	MarkupState state;
	MarkupState_Init(&state, markup);
	MarkupState_UpdatePositions(&state, GAME->windowRect);
	MarkupState_UpdateElements(&state);

	XErr result = XOK;
	Events evs;
	while (true) {
		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (Events_Gather(&evs)) {
			if (evs.quitEvent) {
				result = XERR_QUIT;
				break;
			}
			if (evs.windowResizeEvent) {
				Game_UpdateWindowDimensions(evs.windowDims.x, evs.windowDims.y);
				MarkupState_UpdatePositions(&state, GAME->windowRect);
			}
			CfgMarkupButtonType pressedButton;
			if (MarkupState_HandleEvents(&state, &evs, &pressedButton)) {
				LOGXV_INF(XOK_BUTTON, Int32, pressedButton);
				if (outPressedButton) {
					*outPressedButton = pressedButton;
				}
				result = XOK;
				break;
			}
		}
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Draw markup
		MarkupState_UpdateElements(&state);
		MarkupState_Draw(&state);
		// Present
		SDL_RenderPresent(GAME->sdlRenderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////
	}

	MarkupState_Term(&state);
	return result;
}
