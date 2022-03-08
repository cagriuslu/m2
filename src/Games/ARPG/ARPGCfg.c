#include "ARPGCfg.h"

////////////////////////////////////////////////////////////////////////
////////////////////////////////// UI //////////////////////////////////
////////////////////////////////////////////////////////////////////////
extern const CfgUIElement ARPG_CFG_UI_ENTRYPOINT_ELEMENT_NEWGAMEBUTTON;
extern const CfgUIElement ARPG_CFG_UI_ENTRYPOINT_ELEMENT_QUITBUTTON;
const CfgUI ARPG_CFG_UI_ENTRYPOINT = {
	.w = 100, .h = 100,
	.backgroundColor = {.r = 20, .g = 20, .b = 20, .a = 255},
	.firstElement = &ARPG_CFG_UI_ENTRYPOINT_ELEMENT_NEWGAMEBUTTON
};
const CfgUIElement ARPG_CFG_UI_ENTRYPOINT_ELEMENT_NEWGAMEBUTTON = {
	.x = 45, .y = 35, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.text = "NEW GAME",
	.buttonType = (CfgUIButtonType)ARPG_CFG_UI_BUTTONTYPE_NEWGAME,
	.keyboardShortcut = SDL_SCANCODE_N,
	.next = &ARPG_CFG_UI_ENTRYPOINT_ELEMENT_QUITBUTTON
};
const CfgUIElement ARPG_CFG_UI_ENTRYPOINT_ELEMENT_QUITBUTTON = {
	.x = 45, .y = 55, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.text = "QUIT",
	.buttonType = CFG_UI_BUTTONTYP_QUIT,
	.keyboardShortcut = SDL_SCANCODE_Q
};
