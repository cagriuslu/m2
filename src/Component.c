#include "Component.h"
#include <string.h>

int ComponentInit(Component* component, uint32_t objectId) {
	memset(component, 0, sizeof(Component));
	component->object = objectId;
	return 0;
}

void ComponentDeinit(Component* component) {

}
