#pragma once
#include <m2g/characters/PlayerCharacter.h>

namespace m2g::object {
	class Player final : public character::PlayerCharacterBase {
	public:
		explicit Player(m2::Object& object);
		static Player& Create(bool thisInstance, std::vector<m2::ObjectId>& multiPlayerObjectIds);
	};
	static_assert(m2::CharacterImpl<Player>);
}
