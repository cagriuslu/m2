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

		// Constructors
		GroupId() = default;
		inline GroupId(Type _type, Instance _instance) : type(_type), instance(_instance) {}
		inline explicit GroupId(const pb::Group& group) : GroupId(group.type(), group.instance()) {}

		// Operators
		inline bool operator==(const GroupId& other) const { return (type == other.type) && (instance == other.instance); }
		inline explicit operator bool() const { return type != m2g::pb::NO_GROUP; }

		struct Hash {
			inline std::size_t operator()(const GroupId& k) const {
				return std::hash<Type>()(k.type) ^ std::hash<Instance>()(k.instance);
			}
		};
	};

	/// Group of objects
	class Group {
		Pool<Id, IndexInGroup_MAX + 1> _members;

	public:
		Group() = default;
		virtual ~Group() = default;

		[[nodiscard]] inline uint64_t member_count() const { return _members.size(); }
		inline IndexInGroup add_member(Id object_id) { return _members.emplace(object_id) & 0xFF; }
		inline void remove_member(IndexInGroup index) { _members.free_index(index); }
	};
}

#endif //M2_GROUP_H
