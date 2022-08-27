#ifndef M2_GROUP_H
#define M2_GROUP_H

#include "Pool.hh"
#include <GroupBlueprint.pb.h>
#include <unordered_map>

namespace m2 {
	using GroupTypeID = uint16_t;
	using GroupInstanceID = uint16_t;
	using IndexInGroup = uint8_t;
	constexpr size_t IndexInGroup_MAX = UINT8_MAX;

	struct GroupID {
		GroupTypeID type{0};
		GroupInstanceID instance{0};

		GroupID() = default;
		GroupID(GroupTypeID _type, GroupInstanceID _instance);
		GroupID(const model::GroupBlueprint& group_blueprint);
		bool operator==(const GroupID& other) const;
	};
	struct GroupIDHasher
	{
		std::size_t operator()(const GroupID& k) const {
			using std::hash;
			return hash<GroupTypeID>()(k.type) ^ (hash<GroupInstanceID>()(k.instance) << 1);
		}
	};

	class Group {
		Pool<ID, IndexInGroup_MAX+1> _members;

	public:
		Group() = default;
		virtual ~Group() = default;

		decltype(_members)& members();

		IndexInGroup add_member(ID object_id);
		void remove_member(IndexInGroup index);
	};
}

#endif //M2_GROUP_H
