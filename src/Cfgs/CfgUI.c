#include "../Cfg.h"

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
