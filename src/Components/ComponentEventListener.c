#include "../Component.h"
#include "../Error.h"
#include <string.h>

int EventListenerComponent_Init(ComponentEventListener* evListener, ID objectId) {
	memset(evListener, 0, sizeof(ComponentEventListener));
	return Component_Init((Component*)evListener, objectId);
}

void EventListenerComponent_Term(ComponentEventListener* evListener) {
	Component_Term((Component*)evListener);
	memset(evListener, 0, sizeof(ComponentEventListener));
}
