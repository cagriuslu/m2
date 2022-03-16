#include <m2/Component.h>
#include <m2/Def.h>

M2Err ComponentMonitor_Init(ComponentMonitor* evListener, ID objectId) {
	memset(evListener, 0, sizeof(ComponentMonitor));
	return Component_Init((Component*)evListener, objectId);
}

void ComponentMonitor_Term(ComponentMonitor* evListener) {
	Component_Term((Component*)evListener);
	memset(evListener, 0, sizeof(ComponentMonitor));
}
