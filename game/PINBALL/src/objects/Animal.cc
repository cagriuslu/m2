#include <pinball/Objects.h>
#include <m2/Game.h>

int64_t AnimalAllocator(MAYBE pinball::pb::Animal_Type type) {
	// TODO
	return m2::CreateObject({}, {}, {}).GetId();
}
void AnimalDeallocator(const int64_t id) {
	M2_LEVEL.objects.Free(id);
}
