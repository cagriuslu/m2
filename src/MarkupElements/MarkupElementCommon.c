#include "../MarkupElement.h"
#include "../Def.h"

XErr MarkupElementHP_UpdateDynamicText(MarkupElementState *state);

XErr MarkupElement_UpdateDynamic(MarkupElementState* state) {
	const CfgMarkupElement* cfg = state->cfg;
	switch (cfg->type) {
		case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT:
		case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_TEXT_BUTTON:
			switch (cfg->textType) {
				case CFG_MARKUP_DYNAMIC_TEXT_TYPE_HP:
					return MarkupElementHP_UpdateDynamicText(state);
				default:
					return XERR_INVALID_CFG_DYNTXTTYP;
			}
		case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE:
		case CFG_MARKUP_ELEMENT_TYP_DYNAMIC_IMAGE_BUTTON:
			switch (cfg->imageType) {
				default:
					return XERR_INVALID_CFG_DYNIMGTYP;
			}
		default:
			return LOGXV_ERR(XERR_IMPLEMENTATION, String, "MarkupElement_UpdateDynamic is called for a non-dynamic element");
	}
}
