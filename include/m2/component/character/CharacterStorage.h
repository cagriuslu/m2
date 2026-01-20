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

		// Attributes

		[[nodiscard]] ShiftedPoolId GetBaseShiftedPoolId() const { return std::get<0>(_storageTuple)->GetShiftedPoolId(); }
		[[nodiscard]] PoolId GetBasePoolId() const { return GetBaseShiftedPoolId() >> gPoolIdShiftCount; }

		template <std::size_t CharacterVariantIndex>
		const std::tuple_element_t<CharacterVariantIndex, StorageTuple>::value_type& GetPoolOfVariant() const {
			return *std::get<CharacterVariantIndex>(_storageTuple);
		}
		template <std::size_t CharacterVariantIndex>
		std::tuple_element_t<CharacterVariantIndex, StorageTuple>::value_type& GetPoolOfVariant() {
			return *std::get<CharacterVariantIndex>(_storageTuple);
		}

		int32_t HashCharacters(int32_t initialValue);
		void UpdateCharacters(const Stopwatch::Duration& delta);

		[[nodiscard]] const Character* GetCharacter(CharacterId) const;
		Character* GetCharacter(CharacterId);

		void FreeCharacter(CharacterId);

		void ClearPools();
	};
}
