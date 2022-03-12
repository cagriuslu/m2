#include "GameEntryPoint.h"
#include "../UI.h"

M2Err GameEntryPoint_Init(GameEntryPoint *gep) {
	memset(gep, 0, sizeof(GameEntryPoint));
	return M2OK;
}

M2Err GameEntryPoint_ExecuteEntryUI(GameEntryPoint *gep) {
	CfgUIButtonType button;
	M2Err result = UI_ExecuteBlocking(gep->entryUi, &button);
	if (result) {
		LOG_ERROR_M2(result);
		return result;
	}
	M2Err handlerResult = gep->entryUiButtonHandler(button);
	if (handlerResult == M2ERR_QUIT) {
		LOG_INFO_M2(handlerResult);
	} else if (handlerResult) {
		LOG_ERROR_M2(handlerResult);
	}
	return handlerResult;
}

M2Err GameEntryPoint_ExecutePauseUI(GameEntryPoint *gep) {
	if (gep->pauseUi) {
		CfgUIButtonType button;
		M2Err result = UI_ExecuteBlocking(gep->pauseUi, &button);
		if (result) {
			LOG_ERROR_M2(result);
			return result;
		}
		if (gep->pauseUiButtonHandler) {
			M2Err handlerResult = gep->pauseUiButtonHandler(button);
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

void GameEntryPoint_Term(GameEntryPoint *gep) {
	if (gep->destructor) {
		gep->destructor(gep);
	}
	memset(gep, 0, sizeof(GameEntryPoint));
}
