#pragma once
#include <m2/component/character/ReflectiveCharacter.h>
#include <m2/reflect/Composite.h>

namespace m2g::character {
	using namespace m2::reflect;

	enum class PlayerCharacterField {
		Default
	};
	using PlayerCharacterState = Struct<
		Field<PlayerCharacterField::Default, Empty>
	>;
	using PlayerCharacterBase = m2::ReflectiveCharacter<PlayerCharacterState>;
}
