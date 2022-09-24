#ifndef M2_GROUP_H
#define M2_GROUP_H

#include "Pool.hh"
#include <Level.pb.h>
#include <unordered_map>

namespace m2 {
	using IndexInGroup = uint8_t;
	constexpr size_t IndexInGroup_MAX = UINT8_MAX;

	struct GroupId {
		using Type = m2g::pb::GroupType;
		using Instance = decltype(std::declval<pb::Group>().instance());

		Type type{};
		Instance instance{};

		GroupId() = default;
		GroupId(Type _type, Instance _instance);
		explicit GroupId(const pb::Group& group);
		bool operator==(const GroupId& other) const;

		operator bool() const;

		struct Hash {
			std::size_t operator()(const GroupId& k) const {
				return std::hash<Type>()(k.type) ^ std::hash<Instance>()(k.instance);
			}
		};
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
