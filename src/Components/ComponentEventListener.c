#include "../Component.h"
#include "../Error.h"
#include <string.h>

int ComponentMonitor_Init(ComponentMonitor* evListener, ID objectId) {
	memset(evListener, 0, sizeof(ComponentMonitor));
	return Component_Init((Component*)evListener, objectId);
}

void ComponentMonitor_Term(ComponentMonitor* evListener) {
	Component_Term((Component*)evListener);
	memset(evListener, 0, sizeof(ComponentMonitor));
}
