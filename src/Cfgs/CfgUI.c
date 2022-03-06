#include "../Cfg.h"

////////////////////////////////////////////////////////////////////////
////////////////////////////// START MENU //////////////////////////////
////////////////////////////////////////////////////////////////////////
extern const CfgUIElement CFG_UI_STARTMENU_ELEMENT_NEWGAMEBUTTON;
extern const CfgUIElement CFG_UI_STARTMENU_ELEMENT_QUITBUTTON;
const CfgUI CFG_UI_STARTMENU = {
	.w = 100, .h = 100,
	.firstElement = &CFG_UI_STARTMENU_ELEMENT_NEWGAMEBUTTON
};
const CfgUIElement CFG_UI_STARTMENU_ELEMENT_NEWGAMEBUTTON = {
	.x = 45, .y = 40, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.text = "NEW GAME",
	.buttonType = CFG_UI_BUTTON_TYPE_NEW_GAME,
	.keyboardShortcut = SDL_SCANCODE_N,
	.next = &CFG_UI_STARTMENU_ELEMENT_QUITBUTTON
};
const CfgUIElement CFG_UI_STARTMENU_ELEMENT_QUITBUTTON = {
	.x = 45, .y = 55, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.text = "QUIT",
	.buttonType = CFG_UI_BUTTON_TYPE_QUIT,
	.keyboardShortcut = SDL_SCANCODE_Q,
	.next = NULL
};

////////////////////////////////////////////////////////////////////////
////////////////////////////// PAUSE MENU //////////////////////////////
////////////////////////////////////////////////////////////////////////
extern const CfgUIElement CFG_UI_PAUSEMENU_ELEMENT_RESUMEBUTTON;
const CfgUI CFG_UI_PAUSEMENU = {
		.w = 100, .h = 100,
		.firstElement = &CFG_UI_PAUSEMENU_ELEMENT_RESUMEBUTTON
};
const CfgUIElement CFG_UI_PAUSEMENU_ELEMENT_RESUMEBUTTON = {
		.x = 45, .y = 25, .w = 10, .h = 10,
		.borderWidth_px = 1,
		.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
		.text = "RESUME",
		.buttonType = CFG_UI_BUTTON_TYPE_RESUME,
		.keyboardShortcut = SDL_SCANCODE_R,
		.next = &CFG_UI_STARTMENU_ELEMENT_NEWGAMEBUTTON
};

////////////////////////////////////////////////////////////////////////
/////////////////////////////// HUD LEFT ///////////////////////////////
////////////////////////////////////////////////////////////////////////
extern const CfgUIElement CFG_UI_HUDLEFT_ELEMENT_HPLABEL;
extern const CfgUIElement CFG_UI_HUDLEFT_ELEMENT_HP;
const CfgUI CFG_UI_HUDLEFT = {
	.w = 19, .h = 72,
	.backgroundColor = {50, 50, 50, 255},
	.firstElement = &CFG_UI_HUDLEFT_ELEMENT_HPLABEL
};
const CfgUIElement CFG_UI_HUDLEFT_ELEMENT_HPLABEL = {
	.x = 4, .y = 50, .w = 11, .h = 2,
	.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT,
	.text = "HP",
	.next = &CFG_UI_HUDLEFT_ELEMENT_HP
};
const CfgUIElement CFG_UI_HUDLEFT_ELEMENT_HP = {
	.x = 4, .y = 52, .w = 11, .h = 2,
	.type = CFG_UI_ELEMENT_TYP_DYNAMIC_TEXT,
	.textType = CFG_UI_DYNAMIC_TEXT_TYPE_HP,
	.next = NULL
};

////////////////////////////////////////////////////////////////////////
/////////////////////////////// HUD RIGHT //////////////////////////////
////////////////////////////////////////////////////////////////////////
const CfgUI CFG_UI_HUDRIGHT = {
	.w = 19, .h = 72,
	.backgroundColor = {50, 50, 50, 255},
	.firstElement = NULL
};
