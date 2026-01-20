#pragma once
#include <m2g/ProxyEx.h>
#include <m2/containers/Pool.h>

namespace m2 {
	class CharacterStorage {
		template <typename Tuple>
		struct WrapTupleInOptionalPool;

		template <typename... Ts>
		struct WrapTupleInOptionalPool<std::tuple<Ts...>> {
			using type = std::tuple<std::optional<Pool<Ts>>...>;
		};

		using StorageTuple = WrapTupleInOptionalPool<m2g::ProxyEx::CharacterVariants>::type;
		StorageTuple _storageTuple;

	public:
		CharacterStorage();

		// Direct Pool access

		template <std::size_t CharacterVariantIndex>
		const std::tuple_element_t<CharacterVariantIndex, StorageTuple>::value_type& GetPoolOfVariant() const {
			return *std::get<CharacterVariantIndex>(_storageTuple);
		}
		template <std::size_t CharacterVariantIndex>
		std::tuple_element_t<CharacterVariantIndex, StorageTuple>::value_type& GetPoolOfVariant() {
			return *std::get<CharacterVariantIndex>(_storageTuple);
		}

		// Direct Character access

		[[nodiscard]] const Character* GetCharacter(CharacterId) const;
		Character* GetCharacter(CharacterId);

		template <typename RetType = std::monostate>
		std::optional<RetType> ForEachCharacter(auto operation) {
			const auto iterator = [&](auto& pool) -> std::optional<RetType> {
				for (Character& chr : *pool) {
					if (std::optional<RetType> opResult = operation(chr)) { return opResult; }
				}
				return std::nullopt;
			};
			std::optional<RetType> retval;
			std::apply([&](auto&... pool) {
				((!retval && true ? (retval = iterator(pool)) : std::optional<RetType>{}), ...);
			}, _storageTuple);
			return retval;
		}

		// Accessors

		[[nodiscard]] ShiftedPoolId GetBaseShiftedPoolId() const { return std::get<0>(_storageTuple)->GetShiftedPoolId(); }
		[[nodiscard]] PoolId GetBasePoolId() const { return GetBaseShiftedPoolId() >> gPoolIdShiftCount; }
		[[nodiscard]] int GetTotalCharacterCount() const;
		int32_t HashCharacters(int32_t hash);

		// Modifiers

		void UpdateCharacters(const Stopwatch::Duration& delta);
		void FreeCharacter(CharacterId);
		void ClearPools();
	};
}
