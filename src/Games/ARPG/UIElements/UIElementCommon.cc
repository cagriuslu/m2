#include "../ARPG_UIElement.hh"
#include "m2/Def.hh"

M2Err UIElementHP_UpdateDynamicText(UIElementState *state);

M2Err UIElement_UpdateDynamic(UIElementState* state) {
	const CfgUIElement* cfg = state->cfg;
	switch (cfg->type) {
		case CFG_UI_ELEMENT_TYP_DYNAMIC_TEXT:
		case CFG_UI_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
			switch (cfg->textType) {
				case CFG_UI_DYNAMIC_TEXT_TYPE_HP:
					return UIElementHP_UpdateDynamicText(state);
				default:
					return M2ERR_INVALID_CFG_DYNTXTTYP;
			}
		case CFG_UI_ELEMENT_TYP_DYNAMIC_IMAGE:
		case CFG_UI_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
			switch (cfg->imageType) {
				default:
					return M2ERR_INVALID_CFG_DYNIMGTYP;
			}
		default:
			return LOG_ERROR_M2V(M2ERR_IMPLEMENTATION, String, "UIElement_UpdateDynamic is called for a non-dynamic element");
	}
}
