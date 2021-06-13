#ifndef EVENT_LISTENER_COMPONENT_H
#define EVENT_LISTENER_COMPONENT_H

#include "Component.h"

typedef struct _EventListenerComponent {
	Component super;
	void (*prePhysics)(struct _EventListenerComponent*);
	void (*postPhysics)(struct _EventListenerComponent*);
	void (*preGraphics)(struct _EventListenerComponent*);
	void (*postGraphics)(struct _EventListenerComponent*);
} EventListenerComponent;

int EventListenerComponentInit(EventListenerComponent* evListener, uint32_t objectId);
void EventListenerComponentDeinit(EventListenerComponent* evListener);

#endif
