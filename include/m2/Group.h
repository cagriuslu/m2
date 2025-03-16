#pragma once
#include <m2/containers/Pool.h>
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

		struct Less {
			inline bool operator()(const GroupId& a, const GroupId& b) const {
				return a.type == b.type ? a.instance < b.instance : a.type < b.type;
			}
		};
	};

	/// Group of objects
	class Group {
		Pool<Id, IndexInGroup_MAX + 1> _members;

	public:
		Group() = default;
		virtual ~Group() = default;

		[[nodiscard]] inline uint64_t member_count() const { return _members.Size(); }
		inline IndexInGroup add_member(Id object_id) { return _members.Emplace(object_id).GetId() & 0xFF; }
		inline void remove_member(IndexInGroup index) { _members.FreeIndex(index); }
	};
}
