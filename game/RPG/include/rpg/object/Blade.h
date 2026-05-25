#pragma once
#include <m2/component/character/CompactCharacter.h>

namespace rpg {
	class BladeCharacter : public m2::CompactCharacter {
	public:
		using CompactCharacter::CompactCharacter;

		void OnUpdate(m2::Stopwatch::Duration);
	};
}
