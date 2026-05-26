#pragma once
#include <m2g/ProxyEx.h>
#include <m2/containers/Pool.h>
#include <Lockstep.pb.h>
#include <Network.pb.h>

namespace m2 {
	class Level;

	class CharacterStorage {
		template <typename Tuple>
		struct WrapElements;
		template <typename... Ts>
		struct WrapElements<std::tuple<Ts...>> {
			static constexpr bool isAllValidCharacter = (CharacterImpl<Ts> && ...);
			using OptionalPools = std::tuple<std::optional<Pool<Ts>>...>;
			using ConstVariant = std::variant<std::monostate, std::reference_wrapper<const Ts>...>;
			using Variant = std::variant<std::monostate, std::reference_wrapper<Ts>...>;
		};
		static_assert(WrapElements<m2g::ProxyEx::CharacterVariants>::isAllValidCharacter);
		using StorageTuple = WrapElements<m2g::ProxyEx::CharacterVariants>::OptionalPools;
		StorageTuple _storageTuple;

	public:
		using ConstCharacterVariant = WrapElements<m2g::ProxyEx::CharacterVariants>::ConstVariant;
		using CharacterVariant = WrapElements<m2g::ProxyEx::CharacterVariants>::Variant;

		CharacterStorage();

		// Accessors

		[[nodiscard]] int GetTotalCharacterCount() const;
		[[nodiscard]] ConstCharacterVariant TryGetCharacter(CharacterId) const;
		[[nodiscard]] std::optional<ObjectId> TryGetOwnerId(CharacterId) const;
		[[nodiscard]] int32_t HashAll(int32_t) const;
		void FillAll(const Pool<Object>&, pb::LockstepDebugStateReport&) const;
		void StoreAll(const Pool<Object>&, pb::TurnBasedServerUpdate&) const;
		[[nodiscard]] std::optional<int> TryCountCards(CharacterId, m2g::pb::CardType) const;
		[[nodiscard]] std::optional<int> TryCountCards(CharacterId, m2g::pb::CardCategory) const;
		[[nodiscard]] std::optional<m2g::pb::CardType> TryGetFirstCardType(CharacterId, m2g::pb::CardCategory) const;
		[[nodiscard]] std::optional<VariableValue> TryGetVariable(CharacterId, m2g::pb::VariableType) const;

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

		[[nodiscard]] CharacterVariant TryGetCharacter(CharacterId);
		void UpdateAll(Stopwatch::Duration delta);
		void DeliverMessage(CharacterId, Interaction interaction);
		void Load(CharacterId, const pb::TurnBasedServerUpdate::ObjectDescriptor&);
		void Free(CharacterId);
		void ClearAll();

		template <std::size_t CharacterVariantIndex>
		std::tuple_element_t<CharacterVariantIndex, StorageTuple>::value_type& GetPoolOfVariant() {
			return *std::get<CharacterVariantIndex>(_storageTuple);
		}

		template <std::size_t CharacterVariantIndex>
		auto* TryGetCharacter(const CharacterId chrId) {
			auto& pool = GetPoolOfVariant<CharacterVariantIndex>();
			return pool.Get(chrId);
		}

		template <std::size_t CharacterVariantIndex>
		void DeliverMessage(const CharacterId chrId, Interaction interaction) {
			if (auto* chr = TryGetCharacter<CharacterVariantIndex>(chrId)) {
				chr->OnMessage(std::move(interaction));
			}
		}

	private:
		[[nodiscard]] ShiftedPoolId GetBaseShiftedPoolId() const { return std::get<0>(_storageTuple)->GetShiftedPoolId(); }
		[[nodiscard]] PoolId GetBasePoolId() const { return GetBaseShiftedPoolId() >> gPoolIdShiftCount; }

		friend class Level;
	};
}
