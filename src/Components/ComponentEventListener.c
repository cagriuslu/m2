#include "../Component.h"
#include "../Def.h"

int ComponentMonitor_Init(ComponentMonitor* evListener, ID objectId) {
	memset(evListener, 0, sizeof(ComponentMonitor));
	return Component_Init((Component*)evListener, objectId);
}

void ComponentMonitor_Term(ComponentMonitor* evListener) {
	Component_Term((Component*)evListener);
	memset(evListener, 0, sizeof(ComponentMonitor));
}
