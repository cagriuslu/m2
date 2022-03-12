#ifndef M2_GAMEPROXY_H
#define M2_GAMEPROXY_H

#include "../Cfg.h"

typedef struct _GameProxy {
	const CfgUI* entryUi;
	M2Err(*entryUiButtonHandler)(CfgUIButtonType);
	const CfgUI* pauseUi;
	M2Err(*pauseUiButtonHandler)(CfgUIButtonType);
	
	void (*destructor)(struct _GameProxy*);
} GameProxy;

M2Err GameProxy_Init(GameProxy *gp);
M2Err GameProxy_InitLauncher(GameProxy *gp);
M2Err GameProxy_InitARPG(GameProxy *gp);

M2Err GameProxy_ExecuteEntryUI(GameProxy *gp);
M2Err GameProxy_ExecutePauseUI(GameProxy *gp);

void GameProxy_Term(GameProxy *gp);

#endif //M2_GAMEPROXY_H
