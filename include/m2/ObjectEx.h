#pragma once
#include <m2/Game.h>
#include <m2/Object.h>

namespace m2 {
	template <std::size_t CharacterVariantIndex>
	auto& AddCharacterToObject(Object& obj) {
		auto it = std::get<CharacterVariantIndex>(M2_LEVEL.GetCharacterStorage()._storageTuple)->Emplace(obj.GetId());
		obj._character_id = it.GetId();
		return *it;
	}
}
