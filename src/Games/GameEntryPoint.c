#include "GameEntryPoint.h"

M2Err GameEntryPoint_Init(GameEntryPoint *gep) {
	memset(gep, 0, sizeof(GameEntryPoint));
	return M2OK;
}

void GameEntryPoint_Term(GameEntryPoint *gep) {
	if (gep->destructor) {
		gep->destructor(gep);
	}
	memset(gep, 0, sizeof(GameEntryPoint));
}
