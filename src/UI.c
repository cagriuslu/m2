#include "UI.h"
#include "Event.h"
#include "Def.h"
#include "UIElement.h"
#include "HashMap.h"
#include "Game.h"

// Helper functions

SDL_Rect _UI_CalculateElementRect(SDL_Rect parentRect, unsigned parentW, unsigned parentH, unsigned childX, unsigned childY, unsigned childW, unsigned childH) {
	float pixelsPreUnitW = (float)parentRect.w / (float)parentW;
	float pixelsPreUnitH = (float)parentRect.h / (float)parentH;
	return (SDL_Rect){
			parentRect.x + (int)roundf((float)childX * pixelsPreUnitW),
			parentRect.y + (int)roundf((float)childY * pixelsPreUnitH),
			(int)roundf((float)childW * pixelsPreUnitW),
			(int)roundf((float)childH * pixelsPreUnitH)
	};
}

SDL_Texture* UI_GenerateFontTexture(const char *text) {
	SDL_Surface *textSurf = TTF_RenderUTF8_Blended(GAME->ttfFont, text, (SDL_Color){255, 255, 255, 255});
	SDL_Texture *texture = SDL_CreateTextureFromSurface(GAME->sdlRenderer, textSurf);
	SDL_FreeSurface(textSurf);
	return texture;
}

M2Err _UI_Draw_BackgroundColor(SDL_Rect rect, SDL_Color color) {
	if (color.a == 0) {
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 0, 0, 0, 255);
	} else {
		SDL_SetRenderDrawColor(GAME->sdlRenderer, color.r, color.g, color.b, color.a);
	}
	SDL_RenderFillRect(GAME->sdlRenderer, &rect);
	return M2OK;
}
M2Err _UI_Draw_Text(SDL_Texture* texture, SDL_Rect rect) {
	int textW = 0, textH = 0;
	SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);
	SDL_Rect textRect = (SDL_Rect) {
			rect.x + rect.w / 2 - textW / 2,
			rect.y + rect.h / 2 - textH / 2,
			textW,
			textH
	};
	SDL_RenderCopy(GAME->sdlRenderer, texture, NULL, &textRect);
	return M2OK;
}

UIElementState* _UIState_FindElementByPixel(UIState *state, Vec2I mousePosition) {
	UIElementState* foundElement = NULL;
	for (UIElementState* elementState = state->firstElement; elementState && (foundElement == NULL); elementState = elementState->next) {
		if (elementState->cfg->type == CFG_UI_ELEMENT_TYP_UI) {
			foundElement = _UIState_FindElementByPixel(elementState->child, mousePosition);
		} else {
			SDL_Point p = (SDL_Point){ mousePosition.x, mousePosition.y };
			foundElement = SDL_PointInRect(&p, &elementState->rect) ? elementState : NULL;
		}
	}
	return foundElement;
}

UIElementState* _UIState_FindElementByKeyboardShortcut(UIState* state, const uint8_t* rawKeyboardState) {
	SDL_Scancode keyboardShortcut = 0;
	UIElementState* foundElement = NULL;
	for (UIElementState* elementState = state->firstElement; elementState && (foundElement == NULL); elementState = elementState->next) {
		switch (elementState->cfg->type) {
			case CFG_UI_ELEMENT_TYP_UI:
				foundElement = _UIState_FindElementByKeyboardShortcut(elementState->child, rawKeyboardState);
				break;
			case CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON:
			case CFG_UI_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
			case CFG_UI_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
			case CFG_UI_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
				keyboardShortcut = elementState->cfg->keyboardShortcut;
				foundElement = keyboardShortcut && rawKeyboardState[keyboardShortcut] ? elementState : NULL;
				break;
			default:
				break;
		}
	}
	return foundElement;
}

void _UIState_ResetDepressedButtons(UIState *state) {
	for (UIElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		switch (elementState->cfg->type) {
			case CFG_UI_ELEMENT_TYP_UI:
				_UIState_ResetDepressedButtons(elementState->child);
				break;
			default:
				elementState->depressed = false;
				break;
		}
	}
}

// UIState methods

M2Err UIState_Init(UIState *state, const CfgUI* cfg) {
	memset(state, 0, sizeof(UIState));
	state->cfg = cfg;
	if (cfg && cfg->firstElement) {
		// Address of the variable to store the UIElementState
		UIElementState** elementStatePtrAddress = &state->firstElement;
		// Iterate over CfgUIElements
		for (const CfgUIElement *cfgUIElement = cfg->firstElement; cfgUIElement; cfgUIElement = cfgUIElement->next) {
			// Create UIElementState
			UIElementState* elementState = calloc(1, sizeof(UIElementState));
			if (elementState) {
				elementState->cfg = cfgUIElement;
				// Create child UIState if the type fits
				if (cfgUIElement->type == CFG_UI_ELEMENT_TYP_UI) {
					UIState* childUIState = calloc(1, sizeof(UIState));
					if (childUIState) {
						UIState_Init(childUIState, cfgUIElement->child);
						elementState->child = childUIState;
					} else {
						return M2ERR_OUT_OF_MEMORY;
					}
				}
				// Store UIElementState back to the address
				*elementStatePtrAddress = elementState;
				elementStatePtrAddress = &elementState->next;
			} else {
				return M2ERR_OUT_OF_MEMORY;
			}
		}
	}
	return M2OK;
}

M2Err UIState_UpdatePositions(UIState *state, SDL_Rect rootRect) {
	state->rect = rootRect;
	for (UIElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		// Update rect
		elementState->rect = _UI_CalculateElementRect(state->rect, state->cfg->w, state->cfg->h, elementState->cfg->x,
													  elementState->cfg->y, elementState->cfg->w, elementState->cfg->h);
		// Update individual element
		switch (elementState->cfg->type) {
			case CFG_UI_ELEMENT_TYP_UI:
				M2ERR_REFLECT(UIState_UpdatePositions(elementState->child, elementState->rect));
				break;
			default:
				break;
		}
	}
	return M2OK;
}

M2Err UIState_UpdateElements(UIState* state) {
	for (UIElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		// Update individual element
		switch (elementState->cfg->type) {
			case CFG_UI_ELEMENT_TYP_UI:
				M2ERR_REFLECT(UIState_UpdateElements(elementState->child));
				break;
			case CFG_UI_ELEMENT_TYP_STATIC_TEXT:
			case CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON:
				if (!elementState->textTexture) {
					elementState->textTexture = UI_GenerateFontTexture(elementState->cfg->text);
				}
				break;
			case CFG_UI_ELEMENT_TYP_DYNAMIC_TEXT:
			case CFG_UI_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
			case CFG_UI_ELEMENT_TYP_DYNAMIC_IMAGE:
			case CFG_UI_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
				UIElement_UpdateDynamic(elementState);
				break;
			default:
				break;
		}
	}
	return M2OK;
}

bool UIState_HandleEvents(UIState *state, Events *evs, CfgUIButtonType *outPressedButton) {
	bool eventOccurred = false;
	SDL_Point mousePosition = {.x = evs->mousePosition.x, .y = evs->mousePosition.y};

	if (evs->buttonsPressed[BUTTON_PRIMARY] && SDL_PointInRect(&mousePosition, &state->rect)) {
		UIElementState* elementUnderMouse = _UIState_FindElementByPixel(state, evs->mousePosition);
		if (elementUnderMouse) {
			elementUnderMouse->depressed = true;
		}
	}

	if (evs->buttonsReleased[BUTTON_PRIMARY]) {
		if (SDL_PointInRect(&mousePosition, &state->rect)) {
			UIElementState* elementUnderMouse = _UIState_FindElementByPixel(state, evs->mousePosition);
			if (elementUnderMouse && elementUnderMouse->depressed) {
				eventOccurred = true;
				if (outPressedButton) {
					*outPressedButton = elementUnderMouse->cfg->buttonType;
				}
			}
		}
		_UIState_ResetDepressedButtons(state);
	}

	{
		UIElementState* keyboardShortcutPressedElement = _UIState_FindElementByKeyboardShortcut(state,
																								evs->rawKeyStates);
		if (keyboardShortcutPressedElement) {
			eventOccurred = true;
			if (outPressedButton) {
				*outPressedButton = keyboardShortcutPressedElement->cfg->buttonType;
			}
		}
	}

	return eventOccurred;
}

M2Err UIState_Draw(UIState *state) {
	_UI_Draw_BackgroundColor(state->rect, state->cfg->backgroundColor);
	// Draw elements
	for (UIElementState* elementState = state->firstElement; elementState; elementState = elementState->next) {
		_UI_Draw_BackgroundColor(elementState->rect, elementState->cfg->backgroundColor);
		// Draw element
		switch (elementState->cfg->type) {
			case CFG_UI_ELEMENT_TYP_UI:
				UIState_Draw(elementState->child);
				break;
			case CFG_UI_ELEMENT_TYP_STATIC_TEXT:
			case CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON:
			case CFG_UI_ELEMENT_TYP_DYNAMIC_TEXT:
			case CFG_UI_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
				_UI_Draw_Text(elementState->textTexture, elementState->rect);
				break;
			case CFG_UI_ELEMENT_TYP_STATIC_IMAGE:
			case CFG_UI_ELEMENT_TYP_STATIC_IMAGE_BUTTON:
			case CFG_UI_ELEMENT_TYP_DYNAMIC_IMAGE:
			case CFG_UI_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
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
	// Draw ui border
	if (state->cfg->borderWidth_px) {
		SDL_SetRenderDrawColor(GAME->sdlRenderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(GAME->sdlRenderer, &state->rect);
	}
	return M2OK;
}

void UIState_Term(UIState *state) {
	// TODO
}

M2Err UI_ExecuteBlocking(const CfgUI *ui, CfgUIButtonType* outPressedButton) {
	UIState state;
	UIState_Init(&state, ui);
	UIState_UpdatePositions(&state, GAME->windowRect);
	UIState_UpdateElements(&state);

	M2Err result = M2OK;
	Events evs;
	while (true) {
		////////////////////////////////////////////////////////////////////////
		//////////////////////////// EVENT HANDLING ////////////////////////////
		////////////////////////////////////////////////////////////////////////
		if (Events_Gather(&evs)) {
			if (evs.quitEvent) {
				result = M2ERR_QUIT;
				break;
			}
			if (evs.windowResizeEvent) {
				Game_UpdateWindowDimensions(evs.windowDims.x, evs.windowDims.y);
				UIState_UpdatePositions(&state, GAME->windowRect);
			}
			CfgUIButtonType pressedButton;
			if (UIState_HandleEvents(&state, &evs, &pressedButton)) {
				LOG_INFO_M2V(M2_BUTTON, Int32, pressedButton);
				if (outPressedButton) {
					*outPressedButton = pressedButton;
				}
				result = M2OK;
				break;
			}
		}
		//////////////////////// END OF EVENT HANDLING /////////////////////////
		////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////////////////////////
		/////////////////////////////// GRAPHICS ///////////////////////////////
		////////////////////////////////////////////////////////////////////////
		// Draw ui
		UIState_UpdateElements(&state);
		UIState_Draw(&state);
		// Present
		SDL_RenderPresent(GAME->sdlRenderer);
		/////////////////////////// END OF GRAPHICS ////////////////////////////
		////////////////////////////////////////////////////////////////////////
	}

	UIState_Term(&state);
	return result;
}
