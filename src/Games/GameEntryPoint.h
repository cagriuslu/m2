#ifndef M2_GAMEENTRYPOINT_H
#define M2_GAMEENTRYPOINT_H

#include "../Cfg.h"

typedef struct _GameEntryPoint {
	const CfgUI* entryUi;
	void (*destructor)(struct _GameEntryPoint*);
} GameEntryPoint;

M2Err GameEntryPoint_Init(GameEntryPoint*);
M2Err GameEntryPoint_InitLauncher(GameEntryPoint*);

void GameEntryPoint_Term(GameEntryPoint*);

#endif //M2_GAMEENTRYPOINT_H
