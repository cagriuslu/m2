#ifndef RPG_GROUP_H
#define RPG_GROUP_H

#include <m2/Group.h>
#include <GroupType.pb.h>

namespace m2g {
	m2::Group* create_group(pb::GroupType group_type);
}

#endif //RPG_GROUP_H
