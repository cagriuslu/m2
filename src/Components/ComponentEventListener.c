#include "../Component.h"
#include "../Error.h"
#include <string.h>

int EventListenerComponentInit(EventListenerComponent* evListener, ID objectId) {
	memset(evListener, 0, sizeof(EventListenerComponent));
	return ComponentInit((Component*)evListener, objectId);
}

void EventListenerComponentDeinit(EventListenerComponent* evListener) {
	ComponentDeinit((Component*)evListener);
	memset(evListener, 0, sizeof(EventListenerComponent));
}
