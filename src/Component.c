#include "Component.h"
#include <string.h>

int ComponentInit(Component* component, uint64_t objectId) {
	memset(component, 0, sizeof(Component));
	component->objId = objectId;
	return 0;
}

void ComponentDeinit(Component* component) {
	memset(component, 0, sizeof(Component));
}
