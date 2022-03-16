#include <m2/Component.h>
#include <m2/Game.h>

M2Err ComponentDefense_Init(ComponentDefense* def, ID objId) {
	memset(def, 0, sizeof(ComponentDefense) + GAME->proxy.componentDefenseDataSize);
	return Component_Init((Component*)def, objId);
}

void ComponentDefense_Term(ComponentDefense* def) {
	Component_Term((Component*) def);
	memset(def, 0, sizeof(ComponentDefense));
}
