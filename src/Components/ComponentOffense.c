#include <m2/Component.h>
#include <m2/Game.h>

M2Err ComponentOffense_Init(ComponentOffense* def, ID objId) {
	memset(def, 0, sizeof(ComponentOffense) + GAME->proxy.componentOffenseDataSize);
	return Component_Init((Component*)def, objId);
}

void ComponentOffense_Term(ComponentOffense* off) {
	Component_Term((Component*)off);
	memset(off, 0, sizeof(ComponentOffense));
}
