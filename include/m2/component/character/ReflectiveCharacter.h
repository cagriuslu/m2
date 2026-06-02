#pragma once
#include <m2/component/Character.h>
#include <m2/reflect/utils/Hash.h>
#include <m2/reflect/utils/Proto.h>

namespace m2 {
	template <reflect::IsReflective ReflectiveT>
	class ReflectiveCharacter {
		ObjectId _ownerId;
		ReflectiveT _storage{};

	protected:
		[[nodiscard]] const ReflectiveT& Get() const { return _storage; }
		ReflectiveT& Mutate() { return _storage; }

	public:
		explicit ReflectiveCharacter(const ObjectId ownerId) : _ownerId(ownerId) {}

		[[nodiscard]] ObjectId GetOwnerId() const { return _ownerId; }

		void OnUpdate(Stopwatch::Duration) {}
		void OnMessage(Interaction) {}
		template <reflect::IsVariantReflective T>
		void OnMessage(T&&) {}

		[[nodiscard]] int32_t Hash(int32_t hash) const {
			return reflect::util::Hash(_storage, hash);
		}
		void Fill(pb::LockstepDebugStateReport::Character& chrReport) const {
			reflect::util::StoreToProto(_storage, *chrReport.mutable_reflective());
		}
		void Store(pb::TurnBasedServerUpdate::ObjectDescriptor&) const {
			throw M2_ERROR("Not implemented");
		}
		void Load(const pb::TurnBasedServerUpdate::ObjectDescriptor&) {
			throw M2_ERROR("Not implemented");
		}

		[[nodiscard]] int CountCards(m2g::pb::CardType) const { return 0; }
		[[nodiscard]] int CountCards(m2g::pb::CardCategory) const { return 0; }
		[[nodiscard]] m2g::pb::CardType GetFirstCardType(m2g::pb::CardCategory) const { return {}; }
		[[nodiscard]] VariableValue GetVariable(m2g::pb::VariableType) const { return {}; }
	};
}
