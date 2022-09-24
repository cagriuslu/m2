#ifndef M2_ITEM_H
#define M2_ITEM_H

#include <Item.pb.h>
#include <vector>
#include <string>

namespace m2 {
	std::vector<pb::Item> load_items(const std::string& items_path);
}

#endif //M2_ITEM_H
