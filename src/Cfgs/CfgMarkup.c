#include "../Cfg.h"

extern const CfgMarkupElement CFG_MARKUP_START_MENU_ELEMENT_NEW_GAME_BUTTON;
extern const CfgMarkupElement CFG_MARKUP_START_MENU_ELEMENT_QUIT_BUTTON;
const CfgMarkup CFG_MARKUP_START_MENU = {
	.w = 100, .h = 100,
	.firstElement = &CFG_MARKUP_START_MENU_ELEMENT_NEW_GAME_BUTTON
};
const CfgMarkupElement CFG_MARKUP_START_MENU_ELEMENT_NEW_GAME_BUTTON = {
	.x = 45, .y = 40, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.type = CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.text = "NEW GAME",
	.buttonType = CFG_MARKUP_BUTTON_TYPE_NEW_GAME,
	.keyboardShortcut = SDL_SCANCODE_N,
	.next = &CFG_MARKUP_START_MENU_ELEMENT_QUIT_BUTTON
};
const CfgMarkupElement CFG_MARKUP_START_MENU_ELEMENT_QUIT_BUTTON = {
	.x = 45, .y = 55, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.type = CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.text = "QUIT",
	.buttonType = CFG_MARKUP_BUTTON_TYPE_QUIT,
	.keyboardShortcut = SDL_SCANCODE_Q,
	.next = NULL
};

extern const CfgMarkupElement CFG_MARKUP_PAUSE_MENU_ELEMENT_RESUME_BUTTON;
const CfgMarkup CFG_MARKUP_PAUSE_MENU = {
		.w = 100, .h = 100,
		.firstElement = &CFG_MARKUP_PAUSE_MENU_ELEMENT_RESUME_BUTTON
};
const CfgMarkupElement CFG_MARKUP_PAUSE_MENU_ELEMENT_RESUME_BUTTON = {
		.x = 45, .y = 25, .w = 10, .h = 10,
		.borderWidth_px = 1,
		.type = CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON,
		.text = "RESUME",
		.buttonType = CFG_MARKUP_BUTTON_TYPE_RESUME,
		.keyboardShortcut = SDL_SCANCODE_R,
		.next = &CFG_MARKUP_START_MENU_ELEMENT_NEW_GAME_BUTTON
};

extern const CfgMarkupElement CFG_MARKUP_HUD_LEFT_ELEMENT_LABEL_HP;
extern const CfgMarkupElement CFG_MARKUP_HUD_LEFT_ELEMENT_HP;
const CfgMarkup CFG_MARKUP_HUD_LEFT = {
	.w = 19, .h = 72,
	.backgroundColor = {50, 50, 50, 255},
	.firstElement = &CFG_MARKUP_HUD_LEFT_ELEMENT_LABEL_HP
};
const CfgMarkupElement CFG_MARKUP_HUD_LEFT_ELEMENT_LABEL_HP = {
	.x = 4, .y = 50, .w = 11, .h = 2,
	.type = CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT,
	.text = "HP",
	.next = &CFG_MARKUP_HUD_LEFT_ELEMENT_HP
};
const CfgMarkupElement CFG_MARKUP_HUD_LEFT_ELEMENT_HP = {
	.x = 4, .y = 52, .w = 11, .h = 2,
	.type = CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT,
	.textType = CFG_MARKUP_DYNAMIC_TEXT_TYPE_HP,
	.next = NULL
};
