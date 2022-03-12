#include "GameProxy.h"
#include "../UI.h"

M2Err GameProxy_Init(GameProxy *gp) {
	memset(gp, 0, sizeof(GameProxy));
	return M2OK;
}

M2Err GameProxy_ExecuteEntryUI(GameProxy *gp) {
	CfgUIButtonType button;
	M2Err result = UI_ExecuteBlocking(gp->entryUi, &button);
	if (result) {
		LOG_ERROR_M2(result);
		return result;
	}
	M2Err handlerResult = gp->entryUiButtonHandler(button);
	if (handlerResult == M2ERR_QUIT) {
		LOG_INFO_M2(handlerResult);
	} else if (handlerResult) {
		LOG_ERROR_M2(handlerResult);
	}
	return handlerResult;
}

M2Err GameProxy_ExecutePauseUI(GameProxy *gp) {
	if (gp->pauseUi) {
		CfgUIButtonType button;
		M2Err result = UI_ExecuteBlocking(gp->pauseUi, &button);
		if (result) {
			LOG_ERROR_M2(result);
			return result;
		}
		if (gp->pauseUiButtonHandler) {
			M2Err handlerResult = gp->pauseUiButtonHandler(button);
			if (handlerResult == M2ERR_QUIT) {
				LOG_INFO_M2(handlerResult);
			} else if (handlerResult) {
				LOG_ERROR_M2(handlerResult);
			}
			return handlerResult;
		}
	}
	return M2OK;
}

void GameProxy_Term(GameProxy *gp) {
	if (gp->destructor) {
		gp->destructor(gp);
	}
	memset(gp, 0, sizeof(GameProxy));
}
