#pragma once
#include <m2/containers/Pool.h>
#include <Level.pb.h>
#include <unordered_map>

namespace m2 {
	using IndexInGroup = uint8_t;
	constexpr size_t IndexInGroup_MAX = UINT8_MAX;

	struct GroupIdentifier {
		using Type = m2g::pb::GroupType;
		using Instance = decltype(std::declval<pb::Group>().instance());

		Type type{};
		Instance instance{};

		// Constructors
		GroupIdentifier() = default;
		GroupIdentifier(Type _type, Instance _instance) : type(_type), instance(_instance) {}
		explicit GroupIdentifier(const pb::Group& group) : GroupIdentifier(group.type(), group.instance()) {}

		// Operators
		bool operator==(const GroupIdentifier& other) const { return (type == other.type) && (instance == other.instance); }
		explicit operator bool() const { return type != m2g::pb::NO_GROUP; }

		struct Less {
			bool operator()(const GroupIdentifier& a, const GroupIdentifier& b) const {
				return a.type == b.type ? a.instance < b.instance : a.type < b.type;
			}
		};
	};

	/// Group of objects
	class Group : Pool<Id, IndexInGroup_MAX + 1> {
	public:
		Group() = default;
		virtual ~Group() = default;

		[[nodiscard]] uint64_t MemberCount() const { return Size(); }
		IndexInGroup AddMember(Id object_id) { return Emplace(object_id).GetId() & 0xFF; }
		void RemoveMember(const IndexInGroup index) { FreeIndex(index); }

		using Pool::Iterator;
		using Pool::begin;
		using Pool::end;
	};
}
