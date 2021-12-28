#include "Markup.h"
#include "Event.h"
#include "Log.h"
#include "HashMap.h"
#include "Game.h"

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

SDL_Texture* _Markup_GenerateFontTexture(const char *text) {
	SDL_Surface *textSurf = TTF_RenderUTF8_Blended(GAME->ttfFont, text, (SDL_Color){255, 255, 255, 255});
	SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textSurf);
	SDL_FreeSurface(textSurf);
	return texture;
}

XErr MarkupState_Init(MarkupState *state, const CfgMarkup* cfg) {
	state->cfg = cfg;
	if (cfg && cfg->firstElement) {
		// Address of the variable to store the MarkupElementState
		MarkupElementState** elementStatePtrAddress = &state->firstElement;
		// Iterate over CfgMarkupElements
		for (const CfgMarkupElement *cfgMarkupElement = cfg->firstElement; cfgMarkupElement; cfgMarkupElement = cfgMarkupElement->next) {
			// Create MarkupElementState
			MarkupElementState* elementState = calloc(1, sizeof(MarkupElementState));
			if (elementState) {
				// Store a pointer to Cfg
				elementState->cfg = cfgMarkupElement;
				// Create child MarkupState if the type fits
				if (cfgMarkupElement->type == CFG_MARKUP_ELEMENT_TYPE_MARKUP) {
					MarkupState* childMarkupState = calloc(1, sizeof(MarkupState));
					if (childMarkupState) {
						MarkupState_Init(childMarkupState, cfgMarkupElement->elementUnion.markup);
						elementState->elementUnion.markup = childMarkupState;
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

XErr MarkupState_Update(MarkupState *state, SDL_Rect rootRect) {
	state->rect = rootRect;
	for (MarkupElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		// Update rect
		elementState->rect = _Markup_CalculateElementRect(state->rect, state->cfg->w, state->cfg->h, elementState->cfg->x, elementState->cfg->y, elementState->cfg->w, elementState->cfg->h);
		// Update individual element
		switch (elementState->cfg->type) {
			case CFG_MARKUP_ELEMENT_TYPE_MARKUP:
				REFLECT_ERROR(MarkupState_Update(elementState->elementUnion.markup, elementState->rect));
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT:
				if (!elementState->elementUnion.staticText.textTexture) {
					elementState->elementUnion.staticText.textTexture = _Markup_GenerateFontTexture(elementState->cfg->elementUnion.staticText.text);
				}
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
				if (!elementState->elementUnion.staticTextButton.textTexture) {
					elementState->elementUnion.staticTextButton.textTexture = _Markup_GenerateFontTexture(elementState->cfg->elementUnion.staticTextButton.text);
				}
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT:
				// TODO
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
				// TODO
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE:
				// TODO
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
				// TODO
				break;
			default:
				break;
		}
	}
	return XOK;
}

MarkupElementState* MarkupState_FindElementByPixel(MarkupState *state, Vec2I mousePosition) {
	MarkupElementState* foundElement = NULL;
	for (MarkupElementState* elementState = state->firstElement; elementState && (foundElement == NULL); elementState = elementState->next) {
		if (elementState->cfg->type == CFG_MARKUP_ELEMENT_TYPE_MARKUP) {
			foundElement = MarkupState_FindElementByPixel(elementState->elementUnion.markup, mousePosition);
		} else {
			SDL_Point p = (SDL_Point){ mousePosition.x, mousePosition.y };
			foundElement = SDL_PointInRect(&p, &elementState->rect) ? elementState : NULL;
		}
	}
	return foundElement;
}

MarkupElementState* MarkupState_FindElementByKeyboardShortcut(MarkupState* state, const uint8_t* rawKeyboardState) {
	SDL_Scancode keyboardShortcut = 0;
	MarkupElementState* foundElement = NULL;
	for (MarkupElementState* elementState = state->firstElement; elementState && (foundElement == NULL); elementState = elementState->next) {
		switch (elementState->cfg->type) {
			case CFG_MARKUP_ELEMENT_TYPE_MARKUP:
				foundElement = MarkupState_FindElementByKeyboardShortcut(elementState->elementUnion.markup, rawKeyboardState);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
				keyboardShortcut = elementState->cfg->elementUnion.staticTextButton.keyboardShortcut;
				foundElement = keyboardShortcut && rawKeyboardState[keyboardShortcut] ? elementState : NULL;
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
				keyboardShortcut = elementState->cfg->elementUnion.staticImageButton.keyboardShortcut;
				foundElement = keyboardShortcut && rawKeyboardState[keyboardShortcut] ? elementState : NULL;
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
				keyboardShortcut = elementState->cfg->elementUnion.dynamicTextButton.keyboardShortcut;
				foundElement = keyboardShortcut && rawKeyboardState[keyboardShortcut] ? elementState : NULL;
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
				keyboardShortcut = elementState->cfg->elementUnion.dynamicImageButton.keyboardShortcut;
				foundElement = keyboardShortcut && rawKeyboardState[keyboardShortcut] ? elementState : NULL;
				break;
			default:
				break;
		}
	}
	return foundElement;
}

void MarkupState_ResetDepressedButtons(MarkupState *state) {
	for (MarkupElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		switch (elementState->cfg->type) {
			case CFG_MARKUP_ELEMENT_TYPE_MARKUP:
				MarkupState_ResetDepressedButtons(elementState->elementUnion.markup);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
				elementState->elementUnion.staticTextButton.depressed = false;
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
				elementState->elementUnion.staticImageButton.depressed = false;
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
				elementState->elementUnion.dynamicTextButton.depressed = false;
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
				elementState->elementUnion.dynamicImageButton.depressed = false;
				break;
			default:
				break;
		}
	}
}

bool MarkupState_GetButtonState(MarkupElementState *elementState) {
	switch (elementState->cfg->type) {
		case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
			return elementState->elementUnion.staticTextButton.depressed;
		case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
			return elementState->elementUnion.staticImageButton.depressed;
		case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
			return elementState->elementUnion.dynamicTextButton.depressed;
		case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
			return elementState->elementUnion.dynamicImageButton.depressed;
		default:
			return false;
	}
}

void MarkupState_Term(MarkupState *state) {
	// TODO
}

XErr Markup_Draw_NoPresent_Text(SDL_Texture* texture, SDL_Rect rect) {
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

XErr Markup_Draw_NoPresent(MarkupState *state) {
	// Clear rect
	SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
	SDL_RenderFillRect(GAME->sdlRenderer, &state->rect);
	// Draw elements
	for (MarkupElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		// Clear rect
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
		SDL_RenderFillRect(GAME->sdlRenderer, &elementState->rect);
		// Draw element
		switch (elementState->cfg->type) {
			case CFG_MARKUP_ELEMENT_TYPE_MARKUP:
				Markup_Draw_NoPresent(elementState->elementUnion.markup);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT:
				Markup_Draw_NoPresent_Text(elementState->elementUnion.staticText.textTexture, elementState->rect);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
				Markup_Draw_NoPresent_Text(elementState->elementUnion.staticTextButton.textTexture, elementState->rect);
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE:
				// TODO
				break;
			case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
				// TODO
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT:
				// TODO
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
				// TODO
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE:
				// TODO
				break;
			case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
				// TODO
				break;
			default:
				break;
		}
		// Draw element border
		if (elementState->cfg->borderWidth_px) {
			if (MarkupState_GetButtonState(elementState) == false) {
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

XErr Markup_ExecuteBlocking(const CfgMarkup *markup, CfgMarkupButtonType* outPressedButton) {
	MarkupState state;
	MarkupState_Init(&state, markup);
	MarkupState_Update(&state, GAME->windowRect);

	Events evs;
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
				MarkupState_Update(&state, GAME->windowRect);
			}
			if (evs.buttonsPressed[BUTTON_PRIMARY]) {
				MarkupElementState* elementUnderMouse = MarkupState_FindElementByPixel(&state, evs.mousePosition);
				if (elementUnderMouse) {
					switch (elementUnderMouse->cfg->type) {
						case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
							elementUnderMouse->elementUnion.staticTextButton.depressed = true;
							break;
						case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
							elementUnderMouse->elementUnion.staticImageButton.depressed = true;
							break;
						case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
							elementUnderMouse->elementUnion.dynamicTextButton.depressed = true;
							break;
						case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
							elementUnderMouse->elementUnion.dynamicImageButton.depressed = true;
							break;
						default:
							break;
					}
				}
			}
			if (evs.buttonsReleased[BUTTON_PRIMARY]) {
				MarkupElementState* elementUnderMouse = MarkupState_FindElementByPixel(&state, evs.mousePosition);
				if (elementUnderMouse) {
					switch (elementUnderMouse->cfg->type) {
						case CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON:
							if (elementUnderMouse->elementUnion.staticTextButton.depressed) {
								LOG_INF("Button pressed");
							}
							break;
						case CFG_MARKUP_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
							if (elementUnderMouse->elementUnion.staticImageButton.depressed) {
								LOG_INF("Button pressed");
							}
							break;
						case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
							if (elementUnderMouse->elementUnion.dynamicTextButton.depressed) {
								LOG_INF("Button pressed");
							}
							break;
						case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
							if (elementUnderMouse->elementUnion.dynamicImageButton.depressed) {
								LOG_INF("Button pressed");
							}
							break;
						default:
							break;
					}
				}
				MarkupState_ResetDepressedButtons(&state);
			}
			{
				MarkupElementState* keyboardShortcutPressedElement = MarkupState_FindElementByKeyboardShortcut(&state, evs.rawKeyStates);
				if (keyboardShortcutPressedElement) {
					LOG_INF("Keyboard shortcut pressed");
				}
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
		Markup_Draw_NoPresent(&state);
		// Present
		SDL_RenderPresent(GAME->sdlRenderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////
	}
	MarkupState_Term(&state);
	return XOK;
}
