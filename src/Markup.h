#ifndef CGAME_MARKUP_H
#define CGAME_MARKUP_H

#include "Event.h"
#include "Cfg.h"
#include "Error.h"
#include <SDL.h>

XErr MarkupState_Init(MarkupState *state, const CfgMarkup* cfg);
XErr MarkupState_UpdatePositions(MarkupState* state, SDL_Rect rootRect);
XErr MarkupState_UpdateElements(MarkupState* state);

bool MarkupState_HandleEvents(MarkupState *state, Events *evs, CfgMarkupButtonType *outPressedButton);
XErr MarkupState_Draw(MarkupState *state);

void MarkupState_Term(MarkupState *state);

// High-level functions
XErr Markup_ExecuteBlocking(const CfgMarkup *markup, CfgMarkupButtonType* outPressedButton);

// Helper functions
SDL_Texture* Markup_GenerateFontTexture(const char* text);

#endif //CGAME_MARKUP_H
