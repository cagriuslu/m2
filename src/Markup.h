#ifndef CGAME_MARKUP_H
#define CGAME_MARKUP_H

#include "Cfg.h"
#include "Error.h"
#include <SDL.h>

XErr MarkupState_Init(MarkupState *state, const CfgMarkup* cfg);

XErr MarkupState_Update(MarkupState* state, SDL_Rect rootRect);
MarkupElementState* MarkupState_FindElementByPixel(MarkupState *state, Vec2I mousePosition);
MarkupElementState* MarkupState_FindElementByKeyboardShortcut(MarkupState* state, const uint8_t* rawKeyboardState);
void MarkupState_ResetDepressedButtons(MarkupState *state);
bool MarkupState_GetButtonState(MarkupElementState *elementState);

void MarkupState_Term(MarkupState *state);

// High-level functions
XErr Markup_ExecuteBlocking(const CfgMarkup *markup, CfgMarkupButtonType* outPressedButton);

#endif //CGAME_MARKUP_H
