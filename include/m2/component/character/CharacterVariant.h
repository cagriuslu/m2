#pragma once
#include "CompactCharacter.h"
#include "FastCharacter.h"

namespace m2 {
	using CharacterVariant = std::variant<CompactCharacter,FastCharacter>;

	// Transformers

	Character& ToCharacterBase(CharacterVariant& v);
	FastCharacter& ToFastCharacter(CharacterVariant& v);
	const FastCharacter& ToFastCharacter(const CharacterVariant& v);
}
