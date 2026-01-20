#pragma once
#include <m2/component/character/CompactCharacter.h>
#include <m2/component/character/FastCharacter.h>
#include <tuple>

namespace m2 {
	/// Due to circular dependencies, Proxy cannot refer to every type. This class is included much later, thus can
	/// include way more types.
	class ProxyEx {
	public:
		// Example
		using CharacterVariants = std::tuple<m2::CompactCharacter, m2::FastCharacter>;
		static constexpr auto CompactCharacterStorageIndex = GetIndexInTuple<CompactCharacter, CharacterVariants>::value;
		static constexpr auto FastCharacterStorageIndex = GetIndexInTuple<FastCharacter, CharacterVariants>::value;
	};
}
