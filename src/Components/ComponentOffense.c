#include "../Component.h"
#include <string.h>

int ComponentOffense_Init(ComponentOffense* def, ID objId) {
	memset(def, 0, sizeof(ComponentOffense));
	return Component_Init((Component*)def, objId);
}

void ComponentOffense_Term(ComponentOffense* off) {
	Component_Term((Component*)off);
	memset(off, 0, sizeof(ComponentOffense));
}
