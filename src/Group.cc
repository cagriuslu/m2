#include <m2/Group.h>

bool m2::GroupID::operator==(const GroupID &other) const {
	return (type == other.type) && (instance == other.instance);
}

m2::Pool<ID, m2::IndexInGroup_MAX+1>& m2::Group::members() {
	return _members;
}

m2::IndexInGroup m2::Group::add_member(ID object_id) {
	auto item = _members.alloc();
	item.first = object_id;
	return static_cast<IndexInGroup>(item.second);
}

void m2::Group::remove_member(IndexInGroup index) {
	_members.free_index(index);
}
