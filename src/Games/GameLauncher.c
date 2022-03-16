#include "m2/GameLauncher.h"
#include "m2/Game.h"

////////////////////////////////////////////////////////////////////////
////////////////////////////////// UI //////////////////////////////////
////////////////////////////////////////////////////////////////////////
typedef enum _LauncherCfgUIButtonType {
	LAUNCHER_CFG_UI_BUTTONTYP_ARPG = CFG_UI_BUTTONTYP_N,
} LauncherCfgUIButtonType;

static const CfgUIElement LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_ARPGBUTTON;
static const CfgUIElement LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_QUITBUTTON;
const CfgUI LAUNCHER_CFG_UI_ENTRYPOINT = {
		.w = 100, .h = 100,
		.backgroundColor = {.r = 20, .g = 20, .b = 20, .a = 255},
		.firstElement = &LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_ARPGBUTTON
};
static const CfgUIElement LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_ARPGBUTTON = {
		.x = 45, .y = 35, .w = 10, .h = 10,
		.borderWidth_px = 1,
		.backgroundColor = {.r = 0, .g = 255, .b = 0, .a = 255},
		.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
		.text = "ARPG",
		.buttonType = (CfgUIButtonType)LAUNCHER_CFG_UI_BUTTONTYP_ARPG,
		.keyboardShortcut = SDL_SCANCODE_A,
		.next = &LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_QUITBUTTON
};
static const CfgUIElement LAUNCHER_CFG_UI_ENTRYPOINT_ELEMENT_QUITBUTTON = {
		.x = 45, .y = 55, .w = 10, .h = 10,
		.borderWidth_px = 1,
		.backgroundColor = {.r = 255, .g = 0, .b = 0, .a = 255},
		.type = CFG_UI_ELEMENT_TYP_STATIC_TEXT_BUTTON,
		.text = "QUIT",
		.buttonType = CFG_UI_BUTTONTYP_QUIT,
		.keyboardShortcut = SDL_SCANCODE_Q
};

////////////////////////////////////////////////////////////////////////
//////////////////////////// BUTTON HANDLER ////////////////////////////
////////////////////////////////////////////////////////////////////////
M2Err Launcher_EntryUIButtonHandler(CfgUIButtonType button) {
	if (button == LAUNCHER_CFG_UI_BUTTONTYP_ARPG) {
		GameProxy_Term(&GAME->proxy); // Terminate yourself
		M2ERR_REFLECT(GameProxy_InitARPG(&GAME->proxy)); // Load ARPG
		return M2ERR_PROXY_CHANGED;
	} else {
		return M2ERR_QUIT;
	}
}
