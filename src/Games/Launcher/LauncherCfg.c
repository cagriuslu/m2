#include "LauncherCfg.h"

////////////////////////////////////////////////////////////////////////
////////////////////////////////// UI //////////////////////////////////
////////////////////////////////////////////////////////////////////////
static const CfgUIElement LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_ARPGBUTTON;
static const CfgUIElement LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_RTSBUTTON;
static const CfgUIElement LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_QUITBUTTON;
const CfgUI LAUNCHER_CFG_UI_ENTRYPOINT = {
	.w = 100, .h = 100,
	.borderWidth_px = 1,
	.backgroundColor = {.r = 20, .g = 20, .b = 20, .a = 255},
	.firstElement = &LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_ARPGBUTTON
};
static const CfgUIElement LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_ARPGBUTTON = {
	.x = 45, .y = 30, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.backgroundColor = {.r = 0, .g = 255, .b = 0, .a = 255},
	.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.text = "ARPG",
	.buttonType = (CfgUIButtonType)LAUNCHER_CFG_UI_BUTTONTYP_ARPG,
	.keyboardShortcut = SDL_SCANCODE_A,
	.next = &LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_RTSBUTTON
};
static const CfgUIElement LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_RTSBUTTON = {
	.x = 45, .y = 45, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.backgroundColor = {.r = 0, .g = 0, .b = 255, .a = 255},
	.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.text = "RTS",
	.buttonType = (CfgUIButtonType)LAUNCHER_CFG_UI_BUTTONTYP_RTS,
	.keyboardShortcut = SDL_SCANCODE_S,
	.next = &LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_QUITBUTTON
};
static const CfgUIElement LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_QUITBUTTON = {
	.x = 45, .y = 60, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.backgroundColor = {.r = 0, .g = 0, .b = 0, .a = 255},
	.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.text = "QUIT",
	.buttonType = CFG_UI_BUTTONTYP_QUIT,
	.keyboardShortcut = SDL_SCANCODE_Q
};
