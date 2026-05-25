#pragma once
#include <m2/component/character/CompactCharacter.h>
#include <m2/component/character/FastCharacter.h>
#include <m2/ProxyEx.h>
#include <m2/Meta.h>
#include "mine/object/Dwarf.h"

namespace m2g {
	class ProxyEx final : public m2::ProxyEx {
	public:
		using CharacterVariants = std::tuple<DwarfCharacter, m2::CompactCharacter, m2::FastCharacter>;
		static constexpr auto DwarfCharacterStorageIndex = m2::GetIndexInTuple<DwarfCharacter, CharacterVariants>::value;
		static constexpr auto CompactCharacterStorageIndex = m2::GetIndexInTuple<m2::CompactCharacter, CharacterVariants>::value;
		static constexpr auto FastCharacterStorageIndex = m2::GetIndexInTuple<m2::FastCharacter, CharacterVariants>::value;
	};
}
