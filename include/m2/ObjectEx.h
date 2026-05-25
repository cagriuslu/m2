#pragma once
#include <m2/Game.h>
#include <m2/Object.h>

namespace m2 {
	template <std::size_t CharacterVariantIndex, typename... Args>
	auto& AddCharacterToObject(Object& obj, Args&&... args) {
		auto it = std::get<CharacterVariantIndex>(M2_LEVEL.GetCharacterStorage()._storageTuple)->Emplace(std::forward<Args>(args)...);
		obj._character_id = it.GetId();
		return *it;
	}
}
