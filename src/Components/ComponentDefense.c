#include "../Component.h"
#include <string.h>

int ComponentDefense_Init(ComponentDefense* def, ID objId) {
	memset(def, 0, sizeof(ComponentDefense));
	return Component_Init((Component*)def, objId);
}

void ComponentDefense_CopyExceptSuper(ComponentDefense* dest, ComponentDefense* src) {
	memcpy((Component*)dest + 1, (Component*)src + 1, sizeof(ComponentDefense) - sizeof(Component));
}

void ComponentDefense_Term(ComponentDefense* def) {
	Component_Term((Component*) def);
	memset(def, 0, sizeof(ComponentDefense));
}
