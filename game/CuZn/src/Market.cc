#include <cuzn/Market.h>
#include <m2/Game.h>

void cuzn::set_market_object_resource_count(m2::Id object_id, m2g::pb::ResourceType resource_type, int resource_count) {
	M2_LEVEL.objects[object_id].character().set_resource(resource_type, m2::F(resource_count));
}
