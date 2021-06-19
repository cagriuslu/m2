#include "../Component.h"
#include <string.h>

int ComponentOffenseInit(ComponentOffense* def, uint64_t objId) {
	memset(def, 0, sizeof(ComponentOffense));
	return ComponentInit((Component*)def, objId);
}

void ComponentOffenseCopyExceptSuper(ComponentOffense* dest, ComponentOffense* src) {
	memcpy((Component*)dest + 1, (Component*)src + 1, sizeof(ComponentOffense) - sizeof(Component));
}

void ComponentOffenseDeinit(ComponentOffense* off) {
	ComponentDeinit((Component*)off);
	memset(off, 0, sizeof(ComponentOffense));
}
