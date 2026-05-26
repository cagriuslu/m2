#pragma once
#include <m2/component/character/CompactCharacter.h>
#include <m2/component/character/FastCharacter.h>
#include <tuple>

namespace m2 {
	/// This class is separated from ProxyEx to resolve circular dependencies.
	class ProxyEx {
	public:
		/// Describes different kinds of characters to the game engine. Based on this description, the game engine will
		/// prepare pools of each kind. It is up to the objects to choose which pool to use for its character. Indexes
		/// of the pools created by the game engine will be the same as the indexes of this tuple.
		using CharacterVariants = std::tuple<m2::CompactCharacter, m2::FastCharacter>; // Example
		static constexpr auto CompactCharacterStorageIndex = GetIndexInTuple<CompactCharacter, CharacterVariants>::value;
		static constexpr auto FastCharacterStorageIndex = GetIndexInTuple<FastCharacter, CharacterVariants>::value;
	};
}
