#include "EventListenerComponent.h"
#include "Error.h"
#include <string.h>

int EventListenerComponentInit(EventListenerComponent* evListener, uint32_t objectId) {
	memset(evListener, 0, sizeof(EventListenerComponent));
	return ComponentInit((Component*)evListener, objectId);
}

void EventListenerComponentDeinit(EventListenerComponent* evListener) {
	ComponentDeinit((Component*)evListener);
}
