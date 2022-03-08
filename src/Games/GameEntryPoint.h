#ifndef M2_GAMEENTRYPOINT_H
#define M2_GAMEENTRYPOINT_H

#include "../Cfg.h"

typedef struct _GameEntryPoint {
	const CfgUI* entryUi;
	M2Err(*entryUiButtonHandler)(CfgUIButtonType);
	const CfgUI* pauseUi;
	M2Err(*pauseUiButtonHandler)(CfgUIButtonType);
	
	void (*destructor)(struct _GameEntryPoint*);
} GameEntryPoint;

M2Err GameEntryPoint_Init(GameEntryPoint*);
M2Err GameEntryPoint_InitLauncher(GameEntryPoint*);
M2Err GameEntryPoint_InitARPG(GameEntryPoint*);

M2Err GameEntryPoint_ExecuteEntryUI(GameEntryPoint*);

void GameEntryPoint_Term(GameEntryPoint*);

#endif //M2_GAMEENTRYPOINT_H
