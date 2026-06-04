#pragma once
#include <m2/component/Character.h>

namespace m2 {
	class EmptyCharacter {
		ObjectId _ownerId;

	public:
		explicit EmptyCharacter(const uint64_t object_id) : _ownerId(object_id) {}

		[[nodiscard]] ObjectId GetOwnerId() const { return _ownerId; }

		void OnUpdate(Stopwatch::Duration) {}
		void OnMessage(Interaction) {}
		template <reflect::IsVariantReflective T>
		void OnMessage(T&&) {}

		[[nodiscard]] int32_t Hash(const int32_t seed) const { return seed; }
		void Fill(pb::LockstepDebugStateReport::Character&) const {}
		void Store(pb::TurnBasedServerUpdate::ObjectDescriptor&) const {}
		void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor&) {}

		[[nodiscard]] int CountCards(m2g::pb::CardType) const { return 0; }
		[[nodiscard]] int CountCards(m2g::pb::CardCategory) const { return 0; }
		[[nodiscard]] m2g::pb::CardType GetFirstCardType(m2g::pb::CardCategory) const { return {}; }
		[[nodiscard]] VariableValue GetVariable(m2g::pb::VariableType) const { return {}; }
	};
	static_assert(CharacterImpl<EmptyCharacter>);
}
