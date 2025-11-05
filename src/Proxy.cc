#include <m2/Proxy.h>
#include <m2g_ItemType.pb.h>

m2::pb::Items m2::Proxy::GenerateDefaultItems() {
	pb::Items itemsEnvelope;
	auto* items = itemsEnvelope.mutable_items();
	items->Add()->set_type(m2g::pb::NO_ITEM);
	return itemsEnvelope;
}
