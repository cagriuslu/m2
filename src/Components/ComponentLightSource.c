#include "../Component.h"
#include "../Main.h"
#include "../Object.h"
#include <string.h>

int ComponentLightSourceInit(ComponentLightSource* light, uint64_t objectId, float lightBoundaryRadius) {
	memset(light, 0, sizeof(ComponentLightSource));
	PROPAGATE_ERROR(ComponentInit(&light->super, objectId));
	Object* obj = BucketGetById(&CurrentLevel()->objects, objectId);
	uint64_t myId = BucketGetId(&CurrentLevel()->lightSources, light);
	light->spatialIterator = SpatialMapAdd(&CurrentLevel()->lightSourceSpatialMap, obj->position, lightBoundaryRadius, &myId);
	return 0;
}

void ComponentLightSourceDeinit(ComponentLightSource* light) {
	SpatialMapRemove(&CurrentLevel()->lightSourceSpatialMap, light->spatialIterator);
	ComponentDeinit(&light->super);
	memset(light, 0, sizeof(ComponentLightSource));
}

void ComponentLightSourceUpdatePosition(ComponentLightSource* light) {
	Object* obj = BucketGetById(&CurrentLevel()->objects, light->super.objId);
	SpatialMapSetPosition(&CurrentLevel()->lightSourceSpatialMap, light->spatialIterator, obj->position);
}
