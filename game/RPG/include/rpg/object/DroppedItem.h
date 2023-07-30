#pragma once
#include <m2g_ItemType.pb.h>
#include "m2/Object.h"
#include <m2/Value.h>

namespace rpg {
	m2::VoidValue create_dropped_item(m2::Object& obj, m2g::pb::ItemType item_type);
}
