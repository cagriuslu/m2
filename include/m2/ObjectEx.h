#pragma once
#include <m2/Game.h>
#include <m2/Object.h>

namespace m2 {
	template <std::size_t CharacterVariantIndex>
	Character& AddCharacterToObject(Object& obj) {
		auto it = M2_LEVEL.GetCharacterStorage().GetPoolOfVariant<CharacterVariantIndex>().Emplace(obj.GetId());
		obj._character_id = it.GetId();
		return *it;
	}
}
