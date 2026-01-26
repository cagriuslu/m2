#pragma once
#include <m2/Meta.h>
#include <array>

namespace m2::pb {
	template <typename ProtoEnum, std::size_t N>
	using PossibleEnumOptions = std::array<ProtoEnum, N>;

	template <typename ValueT, PossibleEnumOptions possibleEnumOptions>
	class PartialEnumLUT {
		using EnumT = decltype(possibleEnumOptions)::value_type;
		static_assert(AreArrayElementsUnique(possibleEnumOptions), "PartialEnumLUT requires unique enum options");

		static constexpr int EnumOptionIndex(const EnumT t) {
			for (int i = 0; i < I(possibleEnumOptions.size()); ++i) { if (possibleEnumOptions[i] == t) { return i; } }
			return -1;
		}

		std::array<ValueT, possibleEnumOptions.size()> _values;

		constexpr explicit PartialEnumLUT(std::array<ValueT, possibleEnumOptions.size()> values) : _values(values) {}

	public:
		template <typename... Args>
		static constexpr auto Create(Args&&... args) {
			int matchCount = 0;
			int index = 0;
			((args.first == possibleEnumOptions[index++] ? ++matchCount : 0), ...);
			if (matchCount != I(possibleEnumOptions.size())) {
				throw M2_ERROR("Mapping isn't one to one");
			}
			return PartialEnumLUT{std::array<ValueT, possibleEnumOptions.size()>{args.second...}};
		}

		template <EnumT enumOption>
		[[nodiscard]] consteval const ValueT& Get() const {
			static_assert(DoesArrayContainElement(possibleEnumOptions, enumOption), "This PartialEnumLUT specialization can't hold the given option");
			return _values[EnumOptionIndex(enumOption)];
		}
		[[nodiscard]] const ValueT* TryGet(const EnumT enumOption) const {
			const auto index = EnumOptionIndex(enumOption);
			if (index == -1) { return nullptr; }
			return &_values[index];
		}
		[[nodiscard]] const ValueT& UnsafeGet(const EnumT enumOption) const {
			return *TryGet(enumOption);
		}
	};
}
