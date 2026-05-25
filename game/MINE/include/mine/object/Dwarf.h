#pragma once
#include <m2/Object.h>
#include <m2/Meta.h>
#include <m2/component/character/FastCharacter.h>

class DwarfCharacter : public m2::FastCharacter {
public:
	using FastCharacter::FastCharacter;

	void OnUpdate(m2::Stopwatch::Duration);
};

m2::void_expected create_dwarf(m2::Object& obj, const m2::VecF& position);
