#include <m2/Group.h>

m2::GroupID::GroupID(m2::GroupTypeID _type, m2::GroupInstanceID _instance) : type(_type), instance(_instance) {}

m2::GroupID::GroupID(const pb::GroupBlueprint& gb) : GroupID(gb.type(), gb.instance()) {}

bool m2::GroupID::operator==(const GroupID &other) const {
	return (type == other.type) && (instance == other.instance);
}

m2::Pool<m2::ID, m2::IndexInGroup_MAX+1>& m2::Group::members() {
	return _members;
}

m2::IndexInGroup m2::Group::add_member(ID object_id) {
	auto item = _members.alloc();
	item.first = object_id;
	return static_cast<IndexInGroup>(item.second & 0xFF);
}

void m2::Group::remove_member(IndexInGroup index) {
	_members.free_index(index);
}
