#ifndef COMPONENT_H
#define COMPONENT_H

#include <stdint.h>

typedef struct _Component {
	uint32_t object;
} Component;

int ComponentInit(Component* component, uint32_t objectId);
void ComponentDeinit(Component* component);

#endif
