#include "GameEntryPoint.h"
#include "../UI.h"

M2Err GameEntryPoint_Init(GameEntryPoint *gep) {
	memset(gep, 0, sizeof(GameEntryPoint));
	return M2OK;
}

M2Err GameEntryPoint_ExecuteEntryUI(GameEntryPoint *gep) {
	CfgUIButtonType launcherButton;
	M2Err result = UI_ExecuteBlocking(gep->entryUi, &launcherButton);
	if (result) {
		LOG_ERROR_M2(result);
		return result;
	}
	M2Err handlerResult = gep->entryUiButtonHandler(launcherButton);
	if (handlerResult) {
		LOG_ERROR_M2(handlerResult);
		return result;
	}
	return M2OK;
}

void GameEntryPoint_Term(GameEntryPoint *gep) {
	if (gep->destructor) {
		gep->destructor(gep);
	}
	memset(gep, 0, sizeof(GameEntryPoint));
}
