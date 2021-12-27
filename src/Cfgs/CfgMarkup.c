#include "../Cfg.h"

const CfgMarkupElement CFG_MARKUP_START_MENU_ELEMENT_NEW_GAME_BUTTON;
const CfgMarkupElement CFG_MARKUP_START_MENU_ELEMENT_QUIT_BUTTON;
const CfgMarkup CFG_MARKUP_START_MENU = {
	.w = 100, .h = 100,
	.borderWidth_px = 0,
	.firstElement = &CFG_MARKUP_START_MENU_ELEMENT_NEW_GAME_BUTTON
};
const CfgMarkupElement CFG_MARKUP_START_MENU_ELEMENT_NEW_GAME_BUTTON = {
	.x = 45, .y = 40, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.type = CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.elementUnion.staticTextButton = {
			.text = "NEW GAME",
			.buttonType = CFG_MARKUP_BUTTON_TYPE_NEW_GAME
	},
	.next = &CFG_MARKUP_START_MENU_ELEMENT_QUIT_BUTTON
};
const CfgMarkupElement CFG_MARKUP_START_MENU_ELEMENT_QUIT_BUTTON = {
	.x = 45, .y = 55, .w = 10, .h = 10,
	.borderWidth_px = 1,
	.type = CFG_MARKUP_ELEMENT_TYP_STATIC_TEXT_BUTTON,
	.elementUnion.staticTextButton = {
			.text = "QUIT",
			.buttonType = CFG_MARKUP_BUTTON_TYPE_QUIT
	},
	.next = NULL
};
