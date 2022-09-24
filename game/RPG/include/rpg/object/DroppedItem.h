#ifndef RPG_DROPPEDITEM_H
#define RPG_DROPPEDITEM_H

#include <ItemType.pb.h>
#include "m2/Object.h"
#include <m2/Value.h>

namespace obj {
	struct DroppedItem : public m2::ObjectImpl {
		const m2g::pb::ItemType item_type;

		explicit DroppedItem(m2g::pb::ItemType item_type);
	};

	m2::VoidValue create_dropped_item(m2::Object& obj, m2g::pb::ItemType item_type);
}

#endif //RPG_DROPPEDITEM_H
