#include "../Component.h"
#include "../Main.h"
#include "../Object.h"
#include <string.h>

int ComponentLightSource_Init(ComponentLightSource* light, ID objectId, float lightBoundaryRadius) {
	memset(light, 0, sizeof(ComponentLightSource));
	PROPAGATE_ERROR(Component_Init(&light->super, objectId));
	Object* obj = Bucket_GetById(&CurrentLevel()->objects, objectId);
	ID myId = Bucket_GetId(&CurrentLevel()->lightSources, light);
	light->spatialIterator = SpatialMap_Add(&CurrentLevel()->lightSourceSpatialMap, obj->position, lightBoundaryRadius, &myId);
	return 0;
}

void ComponentLightSource_Term(ComponentLightSource* light) {
	SpatialMap_Remove(&CurrentLevel()->lightSourceSpatialMap, light->spatialIterator);
	Component_Term(&light->super);
	memset(light, 0, sizeof(ComponentLightSource));
}

void ComponentLightSource_UpdatePosition(ComponentLightSource* light) {
	Object* obj = Bucket_GetById(&CurrentLevel()->objects, light->super.objId);
	SpatialMap_SetPosition(&CurrentLevel()->lightSourceSpatialMap, light->spatialIterator, obj->position);
}
