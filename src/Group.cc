#include <m2/Group.h>

m2::GroupId::GroupId(Type _type, Instance _instance) : type(_type), instance(_instance) {}
m2::GroupId::GroupId(const pb::Group& group) : GroupId(group.type(), group.instance()) {}
bool m2::GroupId::operator==(const GroupId &other) const {
	return (type == other.type) && (instance == other.instance);
}
m2::GroupId::operator bool() const {
	return type != m2g::pb::NO_GROUP;
}

m2::Pool<m2::Id, m2::IndexInGroup_MAX + 1>& m2::Group::members() {
	return _members;
}
m2::IndexInGroup m2::Group::add_member(Id object_id) {
	auto item = _members.alloc();
	item.first = object_id;
	return static_cast<IndexInGroup>(item.second & 0xFF);
}
void m2::Group::remove_member(IndexInGroup index) {
	_members.free_index(index);
}
