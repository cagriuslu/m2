#ifndef M2_GROUP_H
#define M2_GROUP_H

#include "Pool.hh"
#include <GroupBlueprint.pb.h>
#include <unordered_map>

namespace m2 {
	using GroupTypeId = uint16_t;
	using GroupInstanceId = uint16_t;
	using IndexInGroup = uint8_t;
	constexpr size_t IndexInGroup_MAX = UINT8_MAX;

	struct GroupId {
		GroupTypeId type{0};
		GroupInstanceId instance{0};

		GroupId() = default;
		GroupId(GroupTypeId _type, GroupInstanceId _instance);
		GroupId(const pb::GroupBlueprint& group_blueprint);
		bool operator==(const GroupId& other) const;
	};
	struct GroupIdHasher
	{
		std::size_t operator()(const GroupId& k) const {
			using std::hash;
			return hash<GroupTypeId>()(k.type) ^ (hash<GroupInstanceId>()(k.instance) << 1);
		}
	};

	class Group {
		Pool<Id, IndexInGroup_MAX + 1> _members;

	public:
		Group() = default;
		virtual ~Group() = default;

		decltype(_members)& members();

		IndexInGroup add_member(Id object_id);
		void remove_member(IndexInGroup index);
	};
}

#endif //M2_GROUP_H
