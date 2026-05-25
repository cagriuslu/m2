#pragma once
#include <m2g/ProxyEx.h>
#include <m2/containers/Pool.h>
#include <Lockstep.pb.h>
#include <Network.pb.h>

namespace m2 {
	class CharacterStorage {
		template <typename Tuple>
		struct WrapTupleInOptionalPool;

		template <typename... Ts>
		requires (CharacterImpl<Ts> && ...)
		struct WrapTupleInOptionalPool<std::tuple<Ts...>> {
			using type = std::tuple<std::optional<Pool<Ts>>...>;
		};

		using StorageTuple = WrapTupleInOptionalPool<m2g::ProxyEx::CharacterVariants>::type;
		StorageTuple _storageTuple;

	public:
		CharacterStorage();

		// Accessors

		[[nodiscard]] int GetTotalCharacterCount() const;
		[[nodiscard]] std::optional<ObjectId> GetOwnerId(CharacterId) const;
		[[nodiscard]] int32_t HashAll(int32_t) const;
		void FillAll(const Pool<Object>&, pb::LockstepDebugStateReport&) const;
		void StoreAll(const Pool<Object>&, pb::TurnBasedServerUpdate&) const;
		[[nodiscard]] std::optional<int> CountCards(CharacterId, m2g::pb::CardType) const;
		[[nodiscard]] std::optional<int> CountCards(CharacterId, m2g::pb::CardCategory) const;
		[[nodiscard]] std::optional<m2g::pb::CardType> GetFirstCardType(CharacterId, m2g::pb::CardCategory) const;
		[[nodiscard]] std::optional<VariableValue> GetVariable(CharacterId, m2g::pb::VariableType) const;

		template <std::size_t CharacterVariantIndex>
		const std::tuple_element_t<CharacterVariantIndex, StorageTuple>::value_type& GetPoolOfVariant() const {
			return *std::get<CharacterVariantIndex>(_storageTuple);
		}

		template <std::size_t CharacterVariantIndex>
		const auto* TryGetCharacter(const CharacterId chrId) const {
			const auto& pool = GetPoolOfVariant<CharacterVariantIndex>();
			return pool.Get(chrId);
		}

		/// Iterate over owner IDs of every character in order
		template <typename RetType = std::monostate>
		std::optional<RetType> ForEachCharacterOwnerId(auto op) const {
			const auto iterator = [&](const auto& pool) -> std::optional<RetType> {
				for (const auto& chr : *pool) { if (std::optional<RetType> opResult = op(chr.GetOwnerId())) { return opResult; } }
				return std::nullopt;
			};
			std::optional<RetType> retval;
			std::apply([&](const auto&... pool) { ((!retval ? (retval = iterator(pool)) : std::optional<RetType>{}), ...); }, _storageTuple);
			return retval;
		}

		// Modifiers

		void UpdateAll(Stopwatch::Duration delta);
		void DeliverMessage(CharacterId, Interaction);
		void Load(CharacterId, const pb::TurnBasedServerUpdate::ObjectDescriptor&);
		void Free(CharacterId);
		void ClearAll();

	private:
		[[nodiscard]] ShiftedPoolId GetBaseShiftedPoolId() const { return std::get<0>(_storageTuple)->GetShiftedPoolId(); }
		[[nodiscard]] PoolId GetBasePoolId() const { return GetBaseShiftedPoolId() >> gPoolIdShiftCount; }

		template <std::size_t CharacterVariantIndex, typename... Args>
		friend auto& AddCharacterToObject(Object&, Args&&... args);
	};
}
