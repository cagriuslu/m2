#include "../Component.h"
#include <string.h>

int ComponentDefenseInit(ComponentDefense* def, uint32_t objId) {
	memset(def, 0, sizeof(ComponentDefense));
	return ComponentInit((Component*)def, objId);
}

void ComponentDefenseDeinit(ComponentDefense* def) {
	ComponentDeinit((Component*) def);
	memset(def, 0, sizeof(ComponentDefense));
}
