#include "m2/Component.hh"

M2Err Component_Init(Component* component, ID objectId) {
	memset(component, 0, sizeof(Component));
	component->objId = objectId;
	return 0;
}

void Component_Term(Component* component) {
	memset(component, 0, sizeof(Component));
}
