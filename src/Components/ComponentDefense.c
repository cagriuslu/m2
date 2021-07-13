#include "../Component.h"
#include <string.h>

int ComponentDefenseInit(ComponentDefense* def, ID objId) {
	memset(def, 0, sizeof(ComponentDefense));
	return ComponentInit((Component*)def, objId);
}

void ComponentDefenseCopyExceptSuper(ComponentDefense* dest, ComponentDefense* src) {
	memcpy((Component*)dest + 1, (Component*)src + 1, sizeof(ComponentDefense) - sizeof(Component));
}

void ComponentDefenseDeinit(ComponentDefense* def) {
	ComponentDeinit((Component*) def);
	memset(def, 0, sizeof(ComponentDefense));
}
